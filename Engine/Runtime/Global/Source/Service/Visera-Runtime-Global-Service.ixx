module;
#include <Visera-Global.hpp>
export module Visera.Runtime.Global.Service;
#define VISERA_MODULE_NAME "Runtime.Global"
export import Visera.Core.Log;
export import Visera.Core.Types.JSON;
export import Visera.Runtime.Global.Configuration;
       import Visera.Platform;
       import Visera.Core.Containers.Map;
       import Visera.Core.Containers.Set;
       import Visera.Core.Types.Path;
       import Visera.Core.Containers.Array;
       import Visera.Core.Containers.Queue;
       import Visera.Core.Types.String;
       import Visera.Core.Types.Pointer;
       import Visera.Core.Meta.Cast;
       import Visera.Core.Delegate.Unicast;
       import Visera.Core.Delegate.Multicast;

export namespace Visera
{
    export using FServiceName = FString;

    namespace EService
    {
        VISERA_RUNTIME_API inline const FString Input     = "Input";
        VISERA_RUNTIME_API inline const FString Window   = "Window";
        VISERA_RUNTIME_API inline const FString RHI      = "RHI";
        VISERA_RUNTIME_API inline const FString Graphics = "Graphics";
        VISERA_RUNTIME_API inline const FString UI       = "UI";
        VISERA_RUNTIME_API inline const FString Audio    = "Audio";
        VISERA_RUNTIME_API inline const FString Physics2D = "Physics2D";
        VISERA_RUNTIME_API inline const FString AssetHub = "AssetHub";
        VISERA_RUNTIME_API inline const FString Tasks    = "Tasks";
    }

    class IRuntimeService;
    namespace Concepts
    {
        template<typename T> concept
        Service = std::is_class_v<T> &&
                  std::derived_from<T, IRuntimeService>;
    }

    using FServiceRegistry = TMap<FString, TSharedPtr<IRuntimeService>>;

    class VISERA_RUNTIME_API IRuntimeService
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
        GetService(const FString& I_ServiceName) const
        {
            if (!Registry)
            {
                LOG_ERROR("Service {} has no registry!", Name);
                return TWeakPtr<T>();
            }
            auto ServiceIter = Registry->Find(I_ServiceName);
            if (ServiceIter == Registry->end())
            {
                return TWeakPtr<T>();
            }
            // Cast TSharedPtr<IRuntimeService> to TSharedPtr<T>, then create TWeakPtr
            if (auto Casted = Cast<T>(ServiceIter->second))
            {
                return TWeakPtr<T>(Casted);
            }
            return TWeakPtr<T>();
        }

        [[nodiscard]] const FString&
        GetName() const { return Name; }

        [[nodiscard]] const TSet<FString>&
        GetDependencies() const { return Dependencies; }

        [[nodiscard]] FString
        GetRuntimeName() const { return RuntimeName; }

        /** Notify this service of a config change. Called by Runtime or other services. */
        void
        NotifyConfigChanged(const FJSONRoute& I_Route)
        { OnConfigChange.Invoke(I_Route); }

        // Public virtual SetStatus: allows subclasses to override state transition behavior
        // Default implementation calls corresponding OnXXX delegates based on the new status
        virtual Bool SetStatus(EStatus I_NewStatus)
        {
            if (Status == I_NewStatus) { return True; }

            // Call corresponding delegate based on target status
            Bool Success = False;
            switch (I_NewStatus)
            {
            case EStatus::Bootstrapped:
                if (Status != EStatus::Pending)
                {
                    LOG_ERROR("Service {} cannot transition to Bootstrapped from current state {}!",
                              Name, static_cast<Int8>(Status));
                    return False;
                }

                // Check if all dependencies are bootstrapped
                if (Registry)
                {
                    for (const FString& DependencyName : Dependencies)
                    {
                        auto DepIter = Registry->Find(DependencyName);
                        if (DepIter == Registry->end())
                        {
                            LOG_ERROR("Service {} depends on {} which is not registered!",
                                      Name, DependencyName);
                            return False;
                        }

                        auto DepService = DepIter->second;
                        if (!DepService->IsBootstrapped())
                        {
                            LOG_ERROR("Service {} depends on {} which is not bootstrapped! (current status: {})",
                                      Name, DependencyName, static_cast<Int8>(DepService->Status));
                            return False;
                        }
                    }
                }
                else if (!Dependencies.IsEmpty())
                {
                    LOG_ERROR("Service {} has dependencies but no registry available!", Name);
                    return False;
                }
                
                Success = OnBootstrap.Invoke().GetValue();
                break;
            case EStatus::Terminated:
                if (Status != EStatus::Bootstrapped)
                {
                    LOG_ERROR("Service {} cannot transition to Terminated from current state {}!",
                              Name, static_cast<Int8>(Status));
                    return False;
                }
                Success = OnTerminate.Invoke().GetValue();
                break;
            case EStatus::Pending:
                LOG_WARN("Service {} cannot transition back to Pending!", Name);
                return False;
            default:
                LOG_ERROR("Service {} cannot transition to unknown state {}!",
                          Name, static_cast<Int8>(I_NewStatus));
                return False;
            }

            if (Success) { Status = I_NewStatus; }
            
            return Success;
        }

    protected:
        TSet<FString>                Dependencies;
        TUnicastDelegate<Bool(void)> OnBootstrap;
        TUnicastDelegate<Bool(void)> OnTerminate;
        TUnicastDelegate<void(const FJSONRoute&)> OnConfigChange;

        FServiceRegistry* Registry {nullptr}; // Registry pointer set by constructor
        mutable EStatus   Status   {EStatus::Pending};

        [[nodiscard]] const FJSONView&
        GetConfig() const { return ConfigView; }

        /** Set config value at path and notify scope subscribers via OnConfigChange. For internal use by Service subclasses. */
        template<Concepts::JSONRoute RouteType> IRuntimeService&
        SetConfig(const RouteType& I_Route, FStringView I_Value)
        {
            ConfigView.Set(I_Route, I_Value);
            if (OnConfigChangeDelegate) { OnConfigChangeDelegate->Broadcast(FJSONRoute(I_Route.GetRouteString())); }
            return *this;
        }

        template<Concepts::JSONRoute RouteType> IRuntimeService&
        SetConfig(const RouteType& I_Route, Double I_Value)
        {
            ConfigView.Set(I_Route, I_Value);
            if (OnConfigChangeDelegate) { OnConfigChangeDelegate->Broadcast(FJSONRoute(I_Route.GetRouteString())); }
            return *this;
        }

        template<Concepts::JSONRoute RouteType> IRuntimeService&
        SetConfig(const RouteType& I_Route, Int64 I_Value)
        {
            ConfigView.Set(I_Route, I_Value);
            if (OnConfigChangeDelegate) { OnConfigChangeDelegate->Broadcast(FJSONRoute(I_Route.GetRouteString())); }
            return *this;
        }

        template<Concepts::JSONRoute RouteType> IRuntimeService&
        SetConfig(const RouteType& I_Route, Bool I_Value)
        {
            ConfigView.Set(I_Route, I_Value);
            if (OnConfigChangeDelegate) { OnConfigChangeDelegate->Broadcast(FJSONRoute(I_Route.GetRouteString())); }
            return *this;
        }

        template<Concepts::JSONRoute RouteType> IRuntimeService&
        SetConfig(const RouteType& I_Route, const FJSON& I_Value)
        {
            ConfigView.Set(I_Route, I_Value);
            if (OnConfigChangeDelegate) { OnConfigChangeDelegate->Broadcast(FJSONRoute(I_Route.GetRouteString())); }
            return *this;
        }

    private:
        FJSONView                                              ConfigView;
        TMulticastDelegate<const FJSONRoute&>*              OnConfigChangeDelegate {nullptr};
        TMulticastDelegate<const FJSONRoute&>::FHandle      ConfigChangeSubscribeHandle {0};
        FString                                                  RuntimeName;

    public:
        virtual ~IRuntimeService()
        {
            if (OnConfigChangeDelegate && ConfigChangeSubscribeHandle != 0)
            { OnConfigChangeDelegate->Unsubscribe(ConfigChangeSubscribeHandle); }
            switch (Status)
            {
            case EStatus::Pending:
                LOG_WARN("Service {} was NOT bootstrapped!", Name);
                break;
            case EStatus::Bootstrapped:
                LOG_ERROR("Service {} was NOT terminated!", Name);
                break;
            case EStatus::Terminated:
                // Service was properly terminated, nothing to do
                break;
            default: LOG_ERROR("Service {} is in unknown state {}!", Name, static_cast<Int8>(Status)); break;
            }
        }

    private:
        const FString Name;

    public:
        IRuntimeService() = delete;
        explicit IRuntimeService(FString I_Name, FServiceRegistry* I_Registry, FJSONView I_ConfigView,
                               TMulticastDelegate<const FJSONRoute&>* I_OnConfigChange, FStringView I_RuntimeName)
            : Name                  (std::move(I_Name))
            , Registry              (I_Registry)
            , ConfigView            (std::move(I_ConfigView))
            , OnConfigChangeDelegate(I_OnConfigChange)
            , RuntimeName           (I_RuntimeName)
        {
            if (OnConfigChangeDelegate)
            {
                ConfigChangeSubscribeHandle = OnConfigChangeDelegate->Subscribe(
                    [this](const FJSONRoute& I_Route) { OnConfigChange.Invoke(I_Route); });
            }
        }

        IRuntimeService(const IRuntimeService&)			 = delete;
        IRuntimeService& operator=(const IRuntimeService&) = delete;
        IRuntimeService(IRuntimeService&&)				 = delete;
        IRuntimeService& operator=(IRuntimeService&&)      = delete;
    };
}