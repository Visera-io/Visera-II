module;
#include <Visera-Global.hpp>
export module Visera.Runtime;
#define VISERA_MODULE_NAME "Runtime"
export import Visera.Runtime.Global;
export import Visera.Runtime.AssetHub;
export import Visera.Runtime.Audio;
export import Visera.Runtime.Graphics;
export import Visera.Runtime.Input;
export import Visera.Runtime.RHI;
export import Visera.Runtime.Tasks;
export import Visera.Runtime.Window;
       import Visera.Core.Types.Set;
       import Visera.Core.Types.Map;
       import Visera.Core.Types.Array;
       import Visera.Core.Types.Queue;
       import Visera.Core.Types.JSON;
       import Visera.Core.Types.Path;
       import Visera.Core.Types.String;
       import Visera.Core.Types.Pointer;
       import Visera.Core.Types.Optional;
       import Visera.Core.OS.FileSystem;
       import Visera.Core.Meta.Cast;

export namespace Visera
{
    enum class EMode
    {
        Minimal,  // Only essential services
        Full,     // All services
    };

    class VISERA_RUNTIME_API FRuntime
    {
    public:
        TSharedPtr<FInput>    Input;
        TSharedPtr<FWindow>   Window;
        TSharedPtr<FTasks>    Tasks;
        TSharedPtr<FRHI>      RHI;
        TSharedPtr<FAudio>    Audio;
        TSharedPtr<FGraphics> Graphics;
        TSharedPtr<FAssetHub> AssetHub;

        // Get a service from the registry
        template<typename T> [[nodiscard]] TWeakPtr<T>
        GetService(FName I_ServiceName) const
        {
            auto ServiceIter = Registry.Find(I_ServiceName);
            if (ServiceIter == Registry.end())
            {
                return TWeakPtr<T>();
            }
            // Cast TSharedPtr<IGlobalService> to TSharedPtr<T>, then create TWeakPtr
            if (auto Casted = Cast<T>(ServiceIter->second))
            {
                return TWeakPtr<T>(Casted);
            }
            return TWeakPtr<T>();
        }

        // Create with mode (Full or Minimal)
        [[nodiscard]] static TUniquePtr<FRuntime>
        Create(EMode I_Mode = EMode::Full, TOptional<FJSON> I_Config = {})
        {
            TSet<FName> Services;
            if (I_Mode == EMode::Full)
            {
                Services =
                {
                    EName::Input,
                    EName::Window,
                    EName::Tasks,
                    EName::RHI,
                    EName::Audio,
                    EName::Graphics,
                    EName::AssetHub
                };
            }
            else // Minimal
            {
                Services =
                {
                    EName::Tasks,
                    EName::AssetHub
                };
            }
            return Create(Services, I_Config);
        }

        // Create with set of service names
        [[nodiscard]] static TUniquePtr<FRuntime>
        Create(const TSet<FName>& I_Services, TOptional<FJSON> I_Config = {})
        {
            // Use new instead of MakeUnique to access private constructor
            TUniquePtr<FRuntime> Runtime(new FRuntime(I_Services, I_Config));
            if (!Runtime)
            {
                LOG_ERROR("Failed to create Visera Runtime!");
                return nullptr;
            }

            Runtime->Bootstrap();

            return Runtime;
        }

        ~FRuntime()
        {
            Terminate();
            // TSharedPtr automatically manages lifetime, no need to manually unregister
        }

    private:
        FServiceRegistry Registry;
        FJSON            Config; // Global config shared by all services

        explicit FRuntime(const TSet<FName>& I_Services, TOptional<FJSON> I_Config = {})
        {
            // Use provided config or default empty config
            if (I_Config.HasValue())
            {
                Config = std::move(I_Config).GetValue();
                LOG_DEBUG("Runtime config provided: {}", Config.Dump());
            }

            // Register services (all services reference the same global Config)
            if (I_Services.Contains(EName::Input))    { Input    = Register<FInput>    (EName::Input);     }
            if (I_Services.Contains(EName::Window))   { Window   = Register<FWindow>   (EName::Window);    }
            if (I_Services.Contains(EName::Tasks))    { Tasks    = Register<FTasks>    (EName::Tasks);     }
            if (I_Services.Contains(EName::RHI))      { RHI      = Register<FRHI>      (EName::RHI);       }
            if (I_Services.Contains(EName::Audio))    { Audio    = Register<FAudio>    (EName::Audio);     }
            if (I_Services.Contains(EName::Graphics)) { Graphics = Register<FGraphics> (EName::Graphics);  }
            if (I_Services.Contains(EName::AssetHub)) { AssetHub = Register<FAssetHub> (EName::AssetHub);  }
        }

        template<typename T> [[nodiscard]] TSharedPtr<T>
        Register(FName I_ServiceName)
        {
            if (Registry.Contains(I_ServiceName))
            {
                LOG_ERROR("Service {} already exists in this Runtime!", I_ServiceName.GetName());
                return TSharedPtr<T>();
            }
            
            // Create service with Registry and reference to global Config
            // TSharedPtr<IGlobalService> Service = MakeShared<T>(I_ServiceName, &Registry, Config);
            auto Service = MakeShared<T>(I_ServiceName, &Registry, Config);

            Registry.Emplace(I_ServiceName, Service);
            LOG_TRACE("Registered service ({}) : {}.", Registry.GetSize(), I_ServiceName.GetName());
            return Cast<T>(Service);
        }

        void
        Unregister(FName I_ServiceName)
        {
            Registry.Erase(I_ServiceName); // TSharedPtr automatically manages lifetime
        }

        void
        Bootstrap()
        {
            auto SortedServices = TopologicalSort();
            
            if (SortedServices.IsEmpty())
            {
                LOG_FATAL("Cannot bootstrap services due to dependency issues!");
                return;
            }

            for (const auto& Service : SortedServices)
            {
                LOG_DEBUG("Bootstrapping {}.", Service->GetName().GetName());
                if (!Service->SetStatus(IGlobalService::EStatus::Bootstrapped))
                { LOG_FATAL("Failed to bootstrap {}!", Service->GetName().GetName()); }
            }
        }

        void
        Terminate()
        {
            auto SortedServices = TopologicalSort();
            
            if (SortedServices.IsEmpty())
            { LOG_FATAL("Cannot terminate services due to dependency issues!"); }

            // Terminate in reverse order (services that depend on others should terminate first)
            for (auto It = SortedServices.rbegin(); It != SortedServices.rend(); ++It)
            {
                const auto& Service = *It;
                LOG_DEBUG("Terminating {}.", Service->GetName().GetName());
                if (!Service->SetStatus(IGlobalService::EStatus::Terminated))
                { LOG_FATAL("Failed to terminate {}!", Service->GetName().GetName()); }
            }
        }

        TArray<TSharedPtr<IGlobalService>> TopologicalSort()
        {
            auto Result = TArray<TSharedPtr<IGlobalService>>();
            Result.Reserve(Registry.GetSize());

            auto Dependents = TMap<FName, TArray<TSharedPtr<IGlobalService>>>(); // service -> list of services that depend on it
            auto InDegrees = TMap<FName, UInt32>(); // service -> how many dependencies it has

            Bool bMissingDependency = False;
            for (auto& [Name, Service] : Registry)
            {
                InDegrees[Name] = static_cast<UInt32>(Service->GetDependencies().GetSize());

                // For each dependency of this service, add this service as a dependent
                for (const FName& DepName : Service->GetDependencies())
                {
                    if (Registry.Contains(DepName))
                    {
                        Dependents[DepName].PushBack(Service);
                    }
                    else
                    {
                        LOG_ERROR("Service {} depends on unregistered service {}!",
                                  Name.GetName(), DepName.GetName());
                        bMissingDependency = True;
                    }
                }
            }
            if (bMissingDependency)
            { LOG_FATAL("Failed to bootstrap Visera Runtime -- Missing dependency!"); }

            // Initialize queue with services that have no dependencies
            auto Queue = TQueue<TSharedPtr<IGlobalService>>();
            for (auto& [Name, Service] : Registry)
            {
                if (InDegrees[Name] == 0)
                {
                    Queue.Push(Service);
                }
            }

            // Process queue
            while (!Queue.IsEmpty())
            {
                auto Current = Queue.Front();
                Queue.Pop();
                Result.PushBack(Current);

                // Decrease in-degree of all services that depend on Current
                auto CurrentName = Current->GetName();
                auto DependentsIter = Dependents.Find(CurrentName);
                if (DependentsIter != Dependents.end())
                {
                    for (const auto& Dependent : DependentsIter->second)
                    {
                        auto DepName = Dependent->GetName();
                        auto InDegreesIter = InDegrees.Find(DepName);
                        if (InDegreesIter != InDegrees.end())
                        {
                            InDegreesIter->second--;
                            if (InDegreesIter->second == 0)
                            {
                                Queue.Push(Dependent);
                            }
                        }
                    }
                }
            }

            // Check for circular dependencies
            if (Result.GetSize() < Registry.GetSize())
            {
                LOG_FATAL("Circular dependency detected in service dependencies! Only {}/{} services could be sorted.",
                          Result.GetSize(), Registry.GetSize());
                
                // Report which services are in the cycle
                TArray<FName> CycleServices;
                for (auto& [Name, InDegree] : InDegrees)
                {
                    if (InDegree > 0)
                    {
                        CycleServices.PushBack(Name);
                    }
                }
                
                FString CycleMsg = "Services involved in cycle: ";
                for (const FName& Name : CycleServices)
                {
                    CycleMsg += FString::Format("{} ", Name.GetName());
                }
                LOG_FATAL("{}", CycleMsg);
                
                Result.Clear();
            }

            return Result;
        }

        FRuntime(const FRuntime&)            = delete;
        FRuntime& operator=(const FRuntime&) = delete;
        FRuntime(FRuntime&&)                 = delete;
        FRuntime& operator=(FRuntime&&)      = delete;
    };
}