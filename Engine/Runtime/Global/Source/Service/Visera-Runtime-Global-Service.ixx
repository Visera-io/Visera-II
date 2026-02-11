module;
#include <Visera-Global.hpp>
export module Visera.Runtime.Global.Service;
#define VISERA_MODULE_NAME "Runtime.Global"
export import Visera.Core.Log;
export import Visera.Core.Types.Name;
export import Visera.Core.Types.JSON;
       import Visera.Platform;
       import Visera.Core.Containers.Map;
       import Visera.Core.Containers.Set;
       import Visera.Core.Types.Path;
       import Visera.Core.Containers.Array;
       import Visera.Core.Containers.Queue;
       import Visera.Core.Types.String;
       import Visera.Core.Types.Pointer;
       import Visera.Core.Meta.Cast;
       import Visera.Core.OS.FileSystem;
       import Visera.Core.Delegate.Unicast;

export namespace Visera
{
    namespace EName
    {
        VISERA_RUNTIME_API inline const auto
        Input     = FName{"input",       0};
        VISERA_RUNTIME_API inline const auto
        Window    = FName{"window",      0};
        VISERA_RUNTIME_API inline const auto
        RHI       = FName{"rhi",         0};
        VISERA_RUNTIME_API inline const auto
        Graphics  = FName{"graphics",    0};
        VISERA_RUNTIME_API inline const auto
        DebugUI   = FName{"debugui",     0};
        VISERA_RUNTIME_API inline const auto
        Audio     = FName{"audio",       0};
        VISERA_RUNTIME_API inline const auto
        Physics2D = FName{"physics2d",   0};
        VISERA_RUNTIME_API inline const auto
        AssetHub  = FName{"assethub",    0};
        VISERA_RUNTIME_API inline const auto
        Tasks     = FName{"tasks",       0};
    }

    class IGlobalService;
    namespace Concepts
    {
        template<typename T> concept
        Service = std::is_class_v<T> &&
                  std::derived_from<T, IGlobalService>;
    }

    using FServiceRegistry = TMap<FName, TSharedPtr<IGlobalService>>;

    class VISERA_RUNTIME_API IGlobalService
    {
    public:
        enum class EStatus : Int8 { Pending, Bootstrapped, Terminated };

        [[nodiscard]] Bool
        IsPending()      const { return Status == EStatus::Pending; }
        [[nodiscard]] Bool
        IsBootstrapped() const { return Status == EStatus::Bootstrapped; }
        [[nodiscard]] Bool
        IsTerminated()   const { return Status == EStatus::Terminated; }

        // Get a service from the registry (used by services to access dependencies)
        template<Concepts::Service T> [[nodiscard]] TWeakPtr<T>
        GetService(FName I_ServiceName) const
        {
            if (!Registry)
            {
                LOG_ERROR("Service {} has no registry!", Name.GetName());
                return TWeakPtr<T>();
            }
            auto ServiceIter = Registry->Find(I_ServiceName);
            if (ServiceIter == Registry->end())
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

        [[nodiscard]] const FName&
        GetName() const { return Name; }

        [[nodiscard]] const TSet<FName>&
        GetDependencies() const { return Dependencies; }

        [[nodiscard]] FString
        GetRuntimeName() const { return Config.GetString("Runtime", "Unknown"); }

        // Public virtual SetStatus: allows subclasses to override state transition behavior
        // Default implementation calls corresponding OnXXX delegates based on the new status
        virtual Bool SetStatus(EStatus I_NewStatus)
        {
            if (Status == I_NewStatus)
            {
                return True; // Already in the target state
            }

            // Call corresponding delegate based on target status
            Bool Success = False;
            switch (I_NewStatus)
            {
            case EStatus::Bootstrapped:
                if (Status != EStatus::Pending)
                {
                    LOG_ERROR("Service {} cannot transition to Bootstrapped from current state {}!", 
                              Name.GetName(), static_cast<Int8>(Status));
                    return False;
                }
                
                // Check if all dependencies are bootstrapped
                if (Registry)
                {
                    for (const FName& DepName : Dependencies)
                    {
                        auto DepIter = Registry->Find(DepName);
                        if (DepIter == Registry->end())
                        {
                            LOG_ERROR("Service {} depends on {} which is not registered!", 
                                      Name.GetName(), DepName.GetName());
                            return False;
                        }
                        
                        auto DepService = DepIter->second;
                        if (!DepService->IsBootstrapped())
                        {
                            LOG_ERROR("Service {} depends on {} which is not bootstrapped! (current status: {})", 
                                      Name.GetName(), DepName.GetName(), static_cast<Int8>(DepService->Status));
                            return False;
                        }
                    }
                }
                else if (!Dependencies.IsEmpty())
                {
                    LOG_ERROR("Service {} has dependencies but no registry available!", Name.GetName());
                    return False;
                }
                
                Success = OnBootstrap.Invoke().GetValue();
                break;
            case EStatus::Terminated:
                if (Status != EStatus::Bootstrapped)
                {
                    LOG_ERROR("Service {} cannot transition to Terminated from current state {}!", 
                              Name.GetName(), static_cast<Int8>(Status));
                    return False;
                }
                Success = OnTerminate.Invoke().GetValue();
                break;
            case EStatus::Pending:
                LOG_WARN("Service {} cannot transition back to Pending!", Name.GetName());
                return False;
            default:
                LOG_ERROR("Service {} cannot transition to unknown state {}!", 
                          Name.GetName(), static_cast<Int8>(I_NewStatus));
                return False;
            }

            if (Success) { Status = I_NewStatus; }
            
            return Success;
        }

    protected:
        TSet<FName>                  Dependencies;
        TUnicastDelegate<Bool(void)> OnBootstrap;
        TUnicastDelegate<Bool(void)> OnTerminate;

        FServiceRegistry* Registry {nullptr}; // Registry pointer set by constructor
        const   FJSON&    Config;             // Config JSON reference to global config in FRuntime (all services share the same global config)
        mutable EStatus   Status   {EStatus::Pending};

    public:
        virtual ~IGlobalService()
        {
            switch (Status)
            {
            case EStatus::Pending:
                LOG_WARN("Service {} was NOT bootstrapped!", Name.GetName());
                break;
            case EStatus::Bootstrapped:
                LOG_ERROR("Service {} was NOT terminated! -- will try to terminate it!", Name.GetName());
                if (!OnTerminate.Invoke())
                { LOG_ERROR("Failed to Terminate Service {}！", Name.GetName()); }
                break;
            case EStatus::Terminated:
                // Service was properly terminated, nothing to do
                break;
            default: LOG_ERROR("Service {} is in unknown statue {} !", Name.GetName(), static_cast<Int8>(Status)); break;
            }
        }

    private:
        const FName Name;

    public:
        IGlobalService() = delete;
        explicit IGlobalService(FName I_Name, FServiceRegistry* I_Registry, const FJSON& I_Config)
            : Name     (I_Name)
            , Registry (I_Registry)
            , Config   (I_Config) { }

        IGlobalService(const IGlobalService&)			 = delete;
        IGlobalService& operator=(const IGlobalService&) = delete;
        IGlobalService(IGlobalService&&)				 = delete;
        IGlobalService& operator=(IGlobalService&&)      = delete;
    };
}