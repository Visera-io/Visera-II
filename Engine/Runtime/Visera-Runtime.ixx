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
       import Visera.Core.Containers.Set;
       import Visera.Core.Containers.Map;
       import Visera.Core.Containers.Array;
       import Visera.Core.Containers.Queue;
       import Visera.Core.Types.JSON;
       import Visera.Core.Types.Path;
       import Visera.Core.Types.String;
       import Visera.Core.Types.Pointer;
       import Visera.Core.Types.Optional;
       import Visera.Core.Meta.Cast;
       import Visera.Core.Log;

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
        [[nodiscard]] FInput*
        GetInput() const { return Input ? Input.Get() : nullptr; }

        [[nodiscard]] FWindow*
        GetWindow() const { return Window ? Window.Get() : nullptr; }

        [[nodiscard]] FTasks*
        GetTasks() const { return Tasks ? Tasks.Get() : nullptr; }

        [[nodiscard]] FRHI*
        GetRHI() const { return RHI ? RHI.Get() : nullptr; }

        [[nodiscard]] FAudio*
        GetAudio() const { return Audio ? Audio.Get() : nullptr; }

        [[nodiscard]] FGraphics*
        GetGraphics() const { return Graphics ? Graphics.Get() : nullptr; }

        [[nodiscard]] FAssetHub*
        GetAssetHub() const { return AssetHub ? AssetHub.Get() : nullptr; }

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

        /** Set config value at path and notify all services via OnConfigChange. */
        template<Concepts::JSONRoute RouteType> FRuntime&
        SetConfig(const RouteType& I_Route, FStringView I_Value)
        {
            LOG_DEBUG("({}) SetConfig: {} = \"{}\"", RuntimeName, I_Route.GetRouteString(), I_Value);

            Config.Set(I_Route, I_Value);
            NotifyConfigChange(I_Route);
            return *this;
        }

        template<Concepts::JSONRoute RouteType> FRuntime&
        SetConfig(const RouteType& I_Route, Double I_Value)
        {
            LOG_DEBUG("({}) SetConfig: {} = \"{}\"", RuntimeName, I_Route.GetRouteString(), I_Value);

            Config.Set(I_Route, I_Value);
            NotifyConfigChange(I_Route);
            return *this;
        }

        template<Concepts::Integral T, Concepts::JSONRoute RouteType> FRuntime&
        SetConfig(const RouteType& I_Route, T I_Value)
        {
            LOG_DEBUG("({}) SetConfig: {} = \"{}\"", RuntimeName, I_Route.GetRouteString(), I_Value);

            Config.Set(I_Route, I_Value);
            NotifyConfigChange(I_Route);
            return *this;
        }

        template<Concepts::Boolean T, Concepts::JSONRoute RouteType> FRuntime&
        SetConfig(const RouteType& I_Route, T I_Value)
        {
            LOG_DEBUG("({}) SetConfig: {} = \"{}\"", RuntimeName, I_Route.GetRouteString(), I_Value);

            Config.Set(I_Route, I_Value);
            NotifyConfigChange(I_Route);
            return *this;
        }

        template<Concepts::JSONRoute RouteType> FRuntime&
        SetConfig(const RouteType& I_Route, const FJSON& I_Value)
        {
            LOG_DEBUG("({}) SetConfig: {} = \"{}\"", RuntimeName, I_Route.GetRouteString(), I_Value);

            Config.Set(I_Route, I_Value);
            NotifyConfigChange(I_Route);
            return *this;
        }

        ~FRuntime()
        {
            Terminate();
            // TSharedPtr automatically manages lifetime, no need to manually unregister
        }

        // Create with mode (Full or Minimal)
        [[nodiscard]] static TUniquePtr<FRuntime>
        Create(FString I_Name = "Runtime", EMode I_Mode = EMode::Full, TOptional<FJSON> I_Config = {})
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
            return Create(std::move(I_Name), Services, I_Config);
        }

        // Create with set of service names
        [[nodiscard]] static TUniquePtr<FRuntime>
        Create(FString I_Name, const TSet<FName>& I_Services, TOptional<FJSON> I_Config = {})
        {
            // Use new instead of MakeUnique to access private constructor
            TUniquePtr<FRuntime> Runtime(new FRuntime(std::move(I_Name), I_Services, I_Config));
            if (!Runtime)
            {
                LOG_ERROR("Failed to create Visera Runtime!");
                return nullptr;
            }

            Runtime->Bootstrap();

            return Runtime;
        }

    private:
        TSharedPtr<FInput>    Input;
        TSharedPtr<FWindow>   Window;
        TSharedPtr<FTasks>    Tasks;
        TSharedPtr<FRHI>      RHI;
        TSharedPtr<FAudio>    Audio;
        TSharedPtr<FGraphics> Graphics;
        TSharedPtr<FAssetHub> AssetHub;

        FServiceRegistry Registry;
        FJSON            Config; // Global config shared by all services
        FString          RuntimeName; // For logging, also in Config["Runtime"]
        TSet<FName>      SharedServices; // Services borrowed from other Runtime instances (Tasks, Audio, AssetHub)

        static inline TWeakPtr<FTasks>    SharedTasks;
        static inline TWeakPtr<FAudio>    SharedAudio;
        static inline TWeakPtr<FAssetHub> SharedAssetHub;
        static inline TWeakPtr<FRHI>      SharedRHI;

        explicit FRuntime(FString I_Name, const TSet<FName>& I_Services, TOptional<FJSON> I_Config = {})
        {
            RuntimeName = std::move(I_Name);
            // Use provided config or default empty config
            if (I_Config.HasValue())
            {
                Config = std::move(I_Config).GetValue();
                LOG_DEBUG("({}) Runtime config provided: {}", RuntimeName, Config.Dump());
            }
            // Inject Runtime name into Config for all services
            Config.Set("Runtime", RuntimeName);

            // Register services (all services reference the same global Config)
            if (I_Services.Contains(EName::Input))
            { Input    = Register<FInput>           (EName::Input);                       }
            if (I_Services.Contains(EName::Window))
            { Window   = Register<FWindow>          (EName::Window);                      }
            if (I_Services.Contains(EName::Tasks))
            { Tasks    = RegisterOrShare<FTasks>    (EName::Tasks,    SharedTasks);    }
            if (I_Services.Contains(EName::RHI))
            { RHI      = RegisterOrShare<FRHI>      (EName::RHI,      SharedRHI);      }
            if (I_Services.Contains(EName::Audio))
            { Audio    = RegisterOrShare<FAudio>    (EName::Audio,    SharedAudio);    }
            if (I_Services.Contains(EName::Graphics))
            { Graphics = Register<FGraphics>        (EName::Graphics);                    }
            if (I_Services.Contains(EName::AssetHub))
            { AssetHub = RegisterOrShare<FAssetHub> (EName::AssetHub, SharedAssetHub); }
        }

        template<typename T> [[nodiscard]] TSharedPtr<T>
        Register(FName I_ServiceName)
        {
            if (Registry.Contains(I_ServiceName))
            {
                LOG_ERROR("Service {} already exists in this Runtime!", I_ServiceName.GetNameString());
                return TSharedPtr<T>();
            }
            
            // Create service with Registry and reference to global Config
            // TSharedPtr<IGlobalService> Service = MakeShared<T>(I_ServiceName, &Registry, Config);
            auto Service = MakeShared<T>(I_ServiceName, &Registry, Config);

            Registry.Insert(I_ServiceName, Service);
            LOG_TRACE("Registered service ({}) : {}.", Registry.GetSize(), I_ServiceName);
            return Cast<T>(Service);
        }

        template<typename T> [[nodiscard]] TSharedPtr<T>
        RegisterOrShare(FName I_ServiceName, TWeakPtr<T>& I_SharedWeak)
        {
            if (Registry.Contains(I_ServiceName))
            {
                LOG_ERROR("Service {} already exists in this Runtime!", I_ServiceName.GetNameString());
                return TSharedPtr<T>();
            }
            if (auto Shared = I_SharedWeak.Lock())
            {
                LOG_INFO("Service {} already created by another Runtime, sharing instance across Runtime instances.", I_ServiceName.GetNameString());
                SharedServices.Insert(I_ServiceName);
                Registry.Insert(I_ServiceName, Cast<IGlobalService>(Shared));
                LOG_TRACE("Registered shared service ({}) : {}.", Registry.GetSize(), I_ServiceName);
                return Shared;
            }
            auto Service = Register<T>(I_ServiceName);
            I_SharedWeak = Service;
            return Service;
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
                LOG_DEBUG("({}) Bootstrapping {}.", RuntimeName, Service->GetName().GetNameString());
                if (!Service->SetStatus(IGlobalService::EStatus::Bootstrapped))
                { LOG_FATAL("({}) Failed to bootstrap {}!", RuntimeName, Service->GetName().GetNameString()); }
            }
            if (Window && RHI)
            { RHI->CreateSwapChain(Window); }
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
                if (SharedServices.Contains(Service->GetName()))
                { continue; } // Shared services are not terminated by this Runtime
                LOG_DEBUG("({}) Terminating {}.", RuntimeName, Service->GetName().GetNameString());
                if (!Service->SetStatus(IGlobalService::EStatus::Terminated))
                { LOG_FATAL("({}) Failed to terminate {}!", RuntimeName, Service->GetName().GetNameString()); }
            }
        }

        template<Concepts::JSONRoute RouteType>
        void
        NotifyConfigChange(const RouteType& I_Route)
        {
            const FJSONRoute DynamicPath{I_Route.GetRouteString()};
            for (auto& [Name, Service] : Registry)
            {
                Service->NotifyConfigChanged(DynamicPath);
            }
        }

        TArray<TSharedPtr<IGlobalService>>
        TopologicalSort()
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
                                  Name.GetNameString(), DepName.GetNameString());
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
                    CycleMsg += FString::Format("{} ", Name.GetNameString());
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