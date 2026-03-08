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
export import Visera.Runtime.UI;
export import Visera.Runtime.Window;
       import Visera.Core.Containers.Set;
       import Visera.Core.Containers.Map;
       import Visera.Core.Containers.Array;
       import Visera.Core.Containers.Queue;
       import Visera.Core.Types.JSON;
       import charted.json;
       import Visera.Core.Types.Path;
       import Visera.Core.Types.String;
       import Visera.Core.Types.Pointer;
       import Visera.Core.Types.Optional;
       import Visera.Core.Meta.Cast;
       import Visera.Core.Log;

namespace Visera
{
    /** Engine mode: Standard = full features (config-driven), Forge = Task, AssetHub, RHI only (Visera-Forge). */
    export enum class EEngineMode
    {
        Standard,
        Forge,
    };

    export class VISERA_RUNTIME_API FViseraApp;
    export class VISERA_RUNTIME_API FViseraEngine;

    class VISERA_RUNTIME_API FViseraEngine
    {
        friend class FViseraApp;
    public:
        [[nodiscard]] FTasks*      GetTasks()    const { return GetGlobalService<FTasks>    (EService::Tasks);    }
        [[nodiscard]] FRHI*        GetRHI()      const { return GetGlobalService<FRHI>      (EService::RHI);      }
        [[nodiscard]] FAudio*      GetAudio()    const { return GetGlobalService<FAudio>    (EService::Audio);    }
        [[nodiscard]] FAssetHub*   GetAssetHub() const { return GetGlobalService<FAssetHub> (EService::AssetHub); }
        [[nodiscard]] FGraphics*   GetGraphics() const { return GetGlobalService<FGraphics> (EService::Graphics); }

        [[nodiscard]] FJSON&       GetConfig()       { return Config.GetRoot(); }
        [[nodiscard]] const FJSON& GetConfig() const { return Config.GetRoot(); }

        /** Create application from JSON config. Bootstrap() is called automatically. */
        [[nodiscard]] FViseraApp*
        CreateApplication(FString I_Name, const FJSON& I_AppConfig);

        explicit FViseraEngine(const FJSON& I_EngineConfig);
        ~FViseraEngine();

        /** Explicitly shut down engine (terminate apps and services). Idempotent. Call before Reset() for clean exit. */
        void
        Terminate();

    private:
        template<typename T> [[nodiscard]] T*
        GetGlobalService(const FString& I_ServiceName) const
        {
            auto it = GlobalRegistry.Find(I_ServiceName);
            if (it == GlobalRegistry.end()) { return nullptr; }
            if (auto p = Cast<T>(it->second)) { return p.Get(); }
            return nullptr;
        }

        template<typename T> [[nodiscard]] TSharedPtr<T>
        RegisterGlobal(const FString& I_ServiceName)
        {
            if (GlobalRegistry.Contains(I_ServiceName))
            {
                LOG_ERROR("Service {} already exists in Engine!", I_ServiceName);
                return TSharedPtr<T>();
            }
            auto EngineConfigView = Config.GetEngineConfig();
            auto Service = MakeShared<T>(I_ServiceName, &GlobalRegistry, std::move(EngineConfigView),
                                         &Config.OnEngineConfigChange, EngineName);
            GlobalRegistry.Insert(I_ServiceName, Service);
            LOG_TRACE("Engine registered service ({}) : {}.", GlobalRegistry.GetSize(), I_ServiceName);
            return Cast<T>(Service);
        }

        void Bootstrap();
        void DestroyApplication(FViseraApp* I_App);

        FString                            EngineName;
        FEngineConfig                      Config;
        FServiceRegistry                   GlobalRegistry;
        TArray<TSharedPtr<IRuntimeService>> GlobalServices;
        TArray<TUniquePtr<FViseraApp>>       CreatedApps;

        FViseraEngine(const FViseraEngine&)            = delete;
        FViseraEngine& operator=(const FViseraEngine&) = delete;
        FViseraEngine(FViseraEngine&&)                 = delete;
        FViseraEngine& operator=(FViseraEngine&&)      = delete;
    };
    
    class VISERA_RUNTIME_API FViseraApp
    {
    public:
        [[nodiscard]] FWindow*   GetWindow()   const { return GetLocalService<FWindow>   (EService::Window);   }
        [[nodiscard]] FInput*    GetInput()    const { return GetLocalService<FInput>    (EService::Input);    }
        [[nodiscard]] FUI*       GetUI()       const { return GetLocalService<FUI>       (EService::UI);       }

        [[nodiscard]] FGraphics* GetGraphics() const { return OwnerEngine->GetGraphics(); }
        [[nodiscard]] FTasks*    GetTasks()    const { return OwnerEngine->GetTasks();    }
        [[nodiscard]] FAudio*    GetAudio()    const { return OwnerEngine->GetAudio();    }
        [[nodiscard]] FAssetHub* GetAssetHub() const { return OwnerEngine->GetAssetHub(); }

        template<Concepts::JSONRoute RouteType> FViseraApp&
        SetConfig(const RouteType& I_Route, FStringView I_Value)
        {
            OwnerEngine->Config.GetAppConfig(AppName).Set(I_Route, I_Value);
            OnConfigChange.Broadcast(FJSONRoute(I_Route.GetRouteString()));
            return *this;
        }
        template<Concepts::JSONRoute RouteType> FViseraApp&
        SetConfig(const RouteType& I_Route, Double I_Value)
        {
            OwnerEngine->Config.GetAppConfig(AppName).Set(I_Route, I_Value);
            OnConfigChange.Broadcast(FJSONRoute(I_Route.GetRouteString()));
            return *this;
        }
        template<Concepts::Integral T, Concepts::JSONRoute RouteType> FViseraApp&
        SetConfig(const RouteType& I_Route, T I_Value)
        {
            OwnerEngine->Config.GetAppConfig(AppName).Set(I_Route, I_Value);
            OnConfigChange.Broadcast(FJSONRoute(I_Route.GetRouteString()));
            return *this;
        }
        template<Concepts::Boolean U, Concepts::JSONRoute RouteType> FViseraApp&
        SetConfig(const RouteType& I_Route, U I_Value)
        {
            OwnerEngine->Config.GetAppConfig(AppName).Set(I_Route, I_Value);
            OnConfigChange.Broadcast(FJSONRoute(I_Route.GetRouteString()));
            return *this;
        }
        template<Concepts::JSONRoute RouteType> FViseraApp&
        SetConfig(const RouteType& I_Route, const FJSON& I_Value)
        {
            OwnerEngine->Config.GetAppConfig(AppName).Set(I_Route, I_Value);
            OnConfigChange.Broadcast(FJSONRoute(I_Route.GetRouteString()));
            return *this;
        }

        /** Bootstrap app services. Called automatically by CreateApplication. */
        void Bootstrap();
        /** Terminate app and remove from Engine. Optional; recommended to just Reset the engine instead. */
        void Terminate();

        ~FViseraApp() { Terminate(); }

    private:
        template<typename T> [[nodiscard]] T*
        GetLocalService(const FString& I_ServiceName) const
        {
            auto it = Registry.Find(I_ServiceName);
            if (it == Registry.end()) { return nullptr; }
            if (auto p = Cast<T>(it->second)) { return p.Get(); }
            return nullptr;
        }

        void InjectEngineServices();
        void RegisterLocalServices();

        template<typename T> [[nodiscard]] TSharedPtr<T>
        RegisterLocal(const FString& I_ServiceName)
        {
            if (Registry.Contains(I_ServiceName))
            {
                LOG_ERROR("Service {} already exists in App!", I_ServiceName);
                return TSharedPtr<T>();
            }
            auto AppConfigView = OwnerEngine->Config.GetAppConfig(AppName);
            auto Service = MakeShared<T>(I_ServiceName, &Registry, std::move(AppConfigView),
                                         &OnConfigChange, AppName);
            Registry.Insert(I_ServiceName, Service);
            LOG_TRACE("App registered service ({}) : {}.", Registry.GetSize(), I_ServiceName);
            return Cast<T>(Service);
        }

        FViseraEngine*                         OwnerEngine {nullptr};
        TMulticastDelegate<const FJSONRoute&>  OnConfigChange;
        FString                                AppName;
        FServiceRegistry     Registry;
        TArray<IRuntimeService*> LocalServicesSorted;  // order only; ownership is in Registry

        explicit FViseraApp(FViseraEngine* I_Owner, FString I_Name);

        FViseraApp(const FViseraApp&)            = delete;
        FViseraApp& operator=(const FViseraApp&) = delete;
        FViseraApp(FViseraApp&&)                 = delete;
        FViseraApp& operator=(FViseraApp&&)      = delete;

        friend class FViseraEngine;
    };

    [[nodiscard]] inline TArray<TSharedPtr<IRuntimeService>>
    TopologicalSort(const FServiceRegistry& I_Registry)
    {
        auto Result = TArray<TSharedPtr<IRuntimeService>>();
        Result.Reserve(I_Registry.GetSize());

        auto Dependents = TMap<FString, TArray<TSharedPtr<IRuntimeService>>>();
        auto InDegrees  = TMap<FString, UInt32>();

        Bool bMissingDependency = False;
        for (auto& [Name, Service] : I_Registry)
        {
            InDegrees[Name] = static_cast<UInt32>(Service->GetDependencies().GetSize());
            for (const FString& DepName : Service->GetDependencies())
            {
                if (I_Registry.Contains(DepName))
                { Dependents[DepName].PushBack(Service); }
                else
                {
                    LOG_ERROR("Service {} depends on unregistered service {}!", Name, DepName);
                    bMissingDependency = True;
                }
            }
        }
        if (bMissingDependency)
        { LOG_FATAL("Failed to bootstrap Visera Runtime -- Missing dependency!"); }

        auto Queue = TQueue<TSharedPtr<IRuntimeService>>();
        for (auto& [Name, Service] : I_Registry)
        {
            if (InDegrees[Name] == 0) { Queue.Push(Service); }
        }

        while (!Queue.IsEmpty())
        {
            auto Current = Queue.Front();
            Queue.Pop();
            Result.PushBack(Current);
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
                        if (InDegreesIter->second == 0) { Queue.Push(Dependent); }
                    }
                }
            }
        }

        if (Result.GetSize() < I_Registry.GetSize())
        {
            LOG_FATAL("Circular dependency detected in service dependencies! Only {}/{} services could be sorted.",
                      Result.GetSize(), I_Registry.GetSize());
            TArray<FString> CycleServices;
            for (auto& [Name, InDegree] : InDegrees)
            { if (InDegree > 0) { CycleServices.PushBack(Name); } }
            FString CycleMsg = "Services involved in cycle: ";
            for (const FString& CycleName : CycleServices)
            { CycleMsg += FString::Format("{} ", CycleName); }
            LOG_FATAL("{}", CycleMsg);
            Result.Clear();
        }
        return Result;
    }

    FViseraEngine::FViseraEngine(const FJSON& I_EngineConfig)
        : Config(I_EngineConfig)
    {
        auto EngineConfig = Config.GetEngineConfig();
        EngineName = EngineConfig.GetString(TJSONRoute<"Name">(), "Visera");
        if (!Config.GetRoot().IsNull()) { LOG_DEBUG("({}) Engine config provided: {}", EngineName, Config.GetRoot().Dump()); }
        EngineConfig.Set(TJSONRoute<"Name">(), EngineName);

        if (EngineConfig.GetBool(TJSONRoute<"Tasks.Enable">(), False))    { (void)RegisterGlobal<FTasks>    (EService::Tasks);    }
        if (EngineConfig.GetBool(TJSONRoute<"RHI.Enable">(), False))      { (void)RegisterGlobal<FRHI>      (EService::RHI);      }
        if (EngineConfig.GetBool(TJSONRoute<"Audio.Enable">(), False))    { (void)RegisterGlobal<FAudio>    (EService::Audio);    }
        if (EngineConfig.GetBool(TJSONRoute<"AssetHub.Enable">(), False)) { (void)RegisterGlobal<FAssetHub> (EService::AssetHub); }
        if (EngineConfig.GetBool(TJSONRoute<"Graphics.Enable">(), False)) { (void)RegisterGlobal<FGraphics> (EService::Graphics); }
        Bootstrap();
    }

    FViseraEngine::~FViseraEngine() { Terminate(); }

    void FViseraEngine::Bootstrap()
    {
        GlobalServices = TopologicalSort(GlobalRegistry);
        if (GlobalServices.IsEmpty())
        { LOG_FATAL("Cannot bootstrap Engine services due to dependency issues!"); }

        for (const auto& Service : GlobalServices)
        {
            LOG_DEBUG("({}) Bootstrapping {}.", EngineName, Service->GetName());
            if (!Service->SetStatus(IRuntimeService::EStatus::Bootstrapped))
            { LOG_FATAL("({}) Failed to bootstrap {}!", EngineName, Service->GetName()); }
        }
    }

    void FViseraEngine::Terminate()
    {
        if (!CreatedApps.IsEmpty())
        { LOG_WARN("({}) Engine terminating with app(s) still registered; terminating them now.", EngineName); }
        while (!CreatedApps.IsEmpty())
        {
            auto* App = CreatedApps[0].Get();
            if (!App) { CreatedApps.Erase(CreatedApps.begin()); continue; }
            LOG_DEBUG("({}) Terminating app {}.", EngineName, App->AppName);
            App->Terminate();
        }
        // Always terminate global services so ~IRuntimeService does not see Bootstrapped (avoids "was NOT terminated!").
        for (auto idx = GlobalServices.GetSize(); idx != 0; )
        {
            --idx;
            const auto& Service = GlobalServices[idx];
            LOG_DEBUG("({}) Terminating {}.", EngineName, Service->GetName());
            if (!Service->SetStatus(IRuntimeService::EStatus::Terminated))
            { LOG_FATAL("({}) Failed to terminate {}!", EngineName, Service->GetName()); }
        }

        CreatedApps.Clear();
        GlobalRegistry.Clear();
        GlobalServices.Clear();
    }

    FViseraApp* FViseraEngine::CreateApplication(FString I_Name, const FJSON& I_AppConfig)
    {
        auto AppPath = FString::Format("{}.{}", kConfigKeyApps, I_Name);
        if (!I_AppConfig.IsNull())
        {
            LOG_DEBUG("({}) Creating app {} with config: {}", EngineName, I_Name, I_AppConfig.Dump());
            Config.GetRoot().Set(FJSONRoute(AppPath.GetNative()), I_AppConfig);
        }
        else
        {
            LOG_WARN("({}) Creating app {} with empty config.", EngineName, I_Name);
            Config.GetRoot().Set(FJSONRoute(AppPath.GetNative()), FJSON{});
        }

        auto App = TUniquePtr<FViseraApp>(new FViseraApp(this, std::move(I_Name)));
        if (!App) return nullptr;
        App->InjectEngineServices();
        App->RegisterLocalServices();
        FViseraApp* Ptr = App.Get();
        CreatedApps.PushBack(std::move(App));
        Ptr->Bootstrap();
        return Ptr;
    }

    void FViseraEngine::DestroyApplication(FViseraApp* I_App)
    {
        if (!I_App) return;
        for (auto It = CreatedApps.begin(); It != CreatedApps.end(); ++It)
        {
            if (It->Get() == I_App)
            {
                TUniquePtr<FViseraApp> Released = std::move(*It);
                CreatedApps.Erase(It);
                return;
            }
        }
    }

    // --- FViseraApp implementation ---

    FViseraApp::FViseraApp(FViseraEngine* I_Owner, FString I_Name)
        : OwnerEngine(I_Owner)
        , AppName   (std::move(I_Name))
    {
    }

    void FViseraApp::InjectEngineServices()
    {
        if (!OwnerEngine) return;
        for (auto& [Name, Service] : OwnerEngine->GlobalRegistry)
        {
            if (Registry.Contains(Name)) continue;
            Registry.Insert(Name, Service);
        }
    }

    void FViseraApp::RegisterLocalServices()
    {
        auto AppConfigView = OwnerEngine->Config.GetAppConfig(AppName);
        if (AppConfigView.GetBool(TJSONRoute<"Window.Enable">(), False)) { (void)RegisterLocal<FWindow> (EService::Window); }
        if (AppConfigView.GetBool(TJSONRoute<"Input.Enable">(), False))  { (void)RegisterLocal<FInput>  (EService::Input);  }
        if (AppConfigView.GetBool(TJSONRoute<"UI.Enable">(), False))     { (void)RegisterLocal<FUI>     (EService::UI);     }
    }

    void FViseraApp::Bootstrap()
    {
        auto Sorted = TopologicalSort(Registry);
        if (Sorted.IsEmpty())
        { LOG_FATAL("({}) Cannot bootstrap App services due to dependency issues!", AppName); }
        LocalServicesSorted.Reserve(Sorted.GetSize());
        for (const auto& P : Sorted)
        { LocalServicesSorted.PushBack(P.Get()); }
        for (IRuntimeService* Service : LocalServicesSorted)
        {
            if (Service->IsBootstrapped()) { continue; }
            LOG_DEBUG("({}) Bootstrapping {}.", AppName, Service->GetName());
            if (!Service->SetStatus(IRuntimeService::EStatus::Bootstrapped))
            { LOG_FATAL("({}) Failed to bootstrap {}!", AppName, Service->GetName()); }
        }
    }

    void FViseraApp::Terminate()
    {
        if (OwnerEngine)
        {
            auto GFX = GetGraphics();
            auto Win = GetWindow();
            if (GFX && Win) { GFX->UnregisterWindow(Win); }
            Registry.EraseIf([this](const FString& K, const TSharedPtr<IRuntimeService>&)
                { return OwnerEngine->GlobalRegistry.Contains(K); });
        }
        for (auto Idx = LocalServicesSorted.GetSize(); Idx != 0; )
        {
            --Idx;
            IRuntimeService* Service = LocalServicesSorted[Idx];
            if (OwnerEngine && OwnerEngine->GlobalRegistry.Contains(Service->GetName())) { continue; }
            if (!Service->IsBootstrapped()) { continue; }
            LOG_DEBUG("({}) Terminating {}.", AppName, Service->GetName());
            if (!Service->SetStatus(IRuntimeService::EStatus::Terminated))
            { LOG_FATAL("({}) Failed to terminate {}!", AppName, Service->GetName()); }
        }
        LocalServicesSorted.Clear();
        if (OwnerEngine)
        {
            OwnerEngine->DestroyApplication(this);
            OwnerEngine = nullptr;
        }
    }
}