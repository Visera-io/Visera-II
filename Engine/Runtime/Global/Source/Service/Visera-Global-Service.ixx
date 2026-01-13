module;
#include <Visera-Global.hpp>
export module Visera.Global.Service;
#define VISERA_MODULE_NAME "Global.Service"
export import Visera.Global.Log;
export import Visera.Global.Name;
       import Visera.Core.Types.Map;
       import Visera.Core.Types.Set;
       import Visera.Core.Types.Array;
       import Visera.Core.Types.Queue;
       import Visera.Core.Delegate.Unicast;

export namespace Visera
{
    namespace EName
    {
        VISERA_GLOBAL_API inline const auto
        Platform  = FName{"platform",    0};
        VISERA_GLOBAL_API inline const auto
        Input     = FName{"input",       0};
        VISERA_GLOBAL_API inline const auto
        Window    = FName{"window",      0};
        VISERA_GLOBAL_API inline const auto
        RHI       = FName{"rhi",         0};
        VISERA_GLOBAL_API inline const auto
        Graphics  = FName{"graphics",    0};
        VISERA_GLOBAL_API inline const auto
        DebugUI   = FName{"debugui",     0};
        VISERA_GLOBAL_API inline const auto
        Audio     = FName{"audio",       0};
        VISERA_GLOBAL_API inline const auto
        Shader    = FName{"shader",      0};
        VISERA_GLOBAL_API inline const auto
        Physics2D = FName{"physics2d",   0};
    }

    class IGlobalService;
    namespace Concepts
    {
        template<typename T> concept
        Service = std::is_class_v<T> &&
                  std::derived_from<T, IGlobalService>;
    }

    class VISERA_GLOBAL_API IGlobalService
    {
        static auto* GetRegistry()
        {
            static auto* Registry = new TMap<FName, IGlobalService*>();
            return Registry;
        }
    public:
        enum class EStatus { Pending, Bootstrapped, Terminated };

        static void
        Bootstrap()
        {
            TArray<IGlobalService*> SortedServices = TopologicalSort();
            
            if (SortedServices.IsEmpty())
            {
                LOG_FATAL("Cannot bootstrap services due to dependency issues!");
                return;
            }

            for (IGlobalService* Service : SortedServices)
            {
                LOG_DEBUG("Bootstrapping {}.", Service->GetDebugName());
                if (!Service->OnBootstrap.Invoke())
                { LOG_FATAL("Failed to bootstrap \"{}\"!", Service->GetDebugName()); }
                Service->Status = EStatus::Bootstrapped;
            }
        }

        static void
        Terminate()
        {
            TArray<IGlobalService*> SortedServices = TopologicalSort();
            
            if (SortedServices.IsEmpty())
            { LOG_FATAL("Cannot terminate services due to dependency issues!"); }

            // Terminate in reverse order (services that depend on others should terminate first)
            for (auto It = SortedServices.rbegin(); It != SortedServices.rend(); ++It)
            {
                IGlobalService* Service = *It;
                LOG_DEBUG("Terminating {}.", Service->GetDebugName());
                if (!Service->OnTerminate.Invoke())
                { LOG_FATAL("Failed to terminate \"{}\"!", Service->GetDebugName()); }
                Service->Status = EStatus::Terminated;
            }
        }

        template<typename T> [[nodiscard]] static T*
        Register(FName I_ServiceName)
        {
            auto Registry = GetRegistry();
            if (Registry->Contains(I_ServiceName))
            {
                LOG_ERROR("Global service {} already exists!", I_ServiceName.GetName());
                return nullptr;
            }
            return static_cast<T*>(Registry->Emplace(I_ServiceName, new T()).first->second);
        }

        template<typename T> [[nodiscard]] static T*
        Get(FName I_ServiceName)
        {
            auto   ServiceIter =  GetRegistry()->Find(I_ServiceName);
            return ServiceIter == GetRegistry()->end() ?
                   nullptr : dynamic_cast<T*>(ServiceIter->second);
        }

        [[nodiscard]] Bool
        IsPending()      const { return Status == EStatus::Pending; }
        [[nodiscard]] Bool
        IsBootstrapped() const { return Status == EStatus::Bootstrapped; }
        [[nodiscard]] Bool
        IsTerminated()   const { return Status == EStatus::Terminated; }

        [[nodiscard]] FStringView
        GetDebugName() const { return Name.GetName(); }

    protected:
        TSet<FName> Dependencies;

        TUnicastDelegate<Bool(void)> OnBootstrap;
        TUnicastDelegate<Bool(void)> OnTerminate;

        // Can only be deleted inside the IGlobalService.
        virtual ~IGlobalService()
        {
            if (IsPending())
            { LOG_WARN("Service \"{}\" was NOT bootstrapped!", GetDebugName()); }
            else if (IsBootstrapped())
            { LOG_ERROR("Service \"{}\" was NOT terminated!", GetDebugName()); }
        }

    private:
        const   FName   Name;
        mutable EStatus Status = EStatus::Pending;

    public:
        IGlobalService() = delete;
        explicit IGlobalService(FName I_Name) : Name(I_Name)
        {
            if(GetRegistry()->Contains(Name))
            { LOG_FATAL("Global service {} already exists!", Name); }

            LOG_TRACE("Registering service ({}) : \"{}\".", GetRegistry()->GetSize() + 1, Name.GetName());
            GetRegistry()->Emplace(Name, this);
        }

        IGlobalService(const IGlobalService&)			 = delete;
        IGlobalService& operator=(const IGlobalService&) = delete;
        IGlobalService(IGlobalService&&)				 = delete;
        IGlobalService& operator=(IGlobalService&&)      = delete;


        static TArray<IGlobalService*>
        TopologicalSort()
        {
            auto* Registry = GetRegistry();
            TArray<IGlobalService*> Result;
            Result.Reserve(Registry->GetSize());

            TMap<FName, TArray<IGlobalService*>> Dependents; // service -> list of services that depend on it
            TMap<FName, UInt32> InDegrees; // service -> how many dependencies it has

            for (auto& [Name, Service] : *Registry)
            {
                InDegrees[Name] = static_cast<UInt32>(Service->Dependencies.GetSize());

                // For each dependency of this service, add this service as a dependent
                for (const FName& DepName : Service->Dependencies)
                {
                    if (Registry->Contains(DepName))
                    {
                        Dependents[DepName].PushBack(Service);
                    }
                    else
                    {
                        LOG_ERROR("Service \"{}\" depends on unregistered service \"{}\"!",
                                  Name.GetName(), DepName.GetName());
                    }
                }
            }

            // Initialize queue with services that have no dependencies
            TQueue<IGlobalService*> Queue;
            for (auto& [Name, Service] : *Registry)
            {
                if (InDegrees[Name] == 0)
                {
                    Queue.Push(Service);
                }
            }

            // Process queue
            while (!Queue.IsEmpty())
            {
                IGlobalService* Current = Queue.Front();
                Queue.Pop();
                Result.PushBack(Current);

                // Decrease in-degree of all services that depend on Current
                FName CurrentName = Current->Name;
                auto DependentsIter = Dependents.Find(CurrentName);
                if (DependentsIter != Dependents.end())
                {
                    for (IGlobalService* Dependent : DependentsIter->second)
                    {
                        FName DepName = Dependent->Name;
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
            if (Result.GetSize() < Registry->GetSize())
            {
                LOG_FATAL("Circular dependency detected in service dependencies! Only {}/{} services could be sorted.",
                          Result.GetSize(), Registry->GetSize());
                
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
                    CycleMsg += Format("\"{}\" ", Name.GetName());
                }
                LOG_FATAL("{}", CycleMsg);
                
                Result.Clear();
            }

            return Result;
        }
    };
}