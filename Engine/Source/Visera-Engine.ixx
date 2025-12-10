module;
#include <Visera-Engine.hpp>
#include <entt/entt.hpp>
export module Visera.Engine;
#define VISERA_MODULE_NAME "Engine"
export import Visera.Engine.AssetHub;
export import Visera.Engine.Audio;
export import Visera.Engine.Render;
export import Visera.Engine.Event;
export import Visera.Engine.World;
export import Visera.Engine.UI;
       import Visera.Core.Types.Name;
       import Visera.Core.Delegate;
       import Visera.Core.OS.Time;
       import Visera.Core.Global;
       import Visera.Core.Log;
       import Visera.Runtime;

namespace Visera
{
    export class VISERA_ENGINE_API FEngine : public IGlobalSingleton<FEngine>
    {
    public:
        TExclusiveUnicastDelegate<Float> AppTick;

        void Run()
        {
            VISERA_ASSERT(IsBootstrapped());

            if (auto Tick = GScripting->GetFunction(PLATFORM_STRING("Tick")))
            {
                if (!AppTick.TryBind([Tick](Float I_DeltaTime)
                {
                    Tick(&I_DeltaTime, sizeof(Float));
                }))
                { LOG_FATAL("Failed to bind the AppTick()!"); }
            }
            else LOG_FATAL("Failed to load the \"AppTick()\" from .NET runtime!");

            if (!GWindow->IsBootstrapped())
            {
                LOG_INFO("Visera Off-Screen Mode.");
                GRHI->BeginFrame();
                {
                    GEvent->OnFrameBegin.Broadcast();
                    AppTick.Invoke(0);
                    GEvent->OnFrameEnd.Broadcast();
                }
                GRHI->EndFrame();
                return;
            }
            
             auto& Driver = GRHI->GetDriver();

            struct alignas(16) FTestPushConstantRange
            {
                Float Time{0};
                Float CursorX{0}, CursorY{0};
                Float OffsetX{0}, OffsetY{0};
            };
            static_assert(sizeof(FTestPushConstantRange) <= 128);
            FTestPushConstantRange MouseContext{};

             while (!GWindow->ShouldClose())
             {
                 GWindow->PollEvents();
                 GAudio->Tick();

                 Float DeltaTime = Timer.Tick().Microseconds() / 1000000.0; Timer.Reset();

                 GRHI->BeginFrame();
                 {
                     GEvent ->OnFrameBegin.Broadcast();

                     // Logic
                     AppTick.Invoke(DeltaTime);
                     GWorld ->Tick(DeltaTime);
                     // Render
                     GRender->Tick(DeltaTime);

                     GEvent ->OnFrameEnd.Broadcast();
                 }
                 GRHI->EndFrame();
                 GRHI->Present();
             }
        }
    private:
        FHiResClock Timer;

    public:
        void Bootstrap() override
        {
            LOG_TRACE("Bootstrapping Engine.");
            GRuntime    ->Bootstrap();

            GEvent      ->Bootstrap();
            GAssetHub   ->Bootstrap();
            GRender     ->Bootstrap();
            GAudio      ->Bootstrap();

            GEvent->OnEngineBootstrap.Broadcast();

            Status = EStatus::Bootstrapped;
        }


        void Terminate() override
        {
            LOG_TRACE("Terminating Engine.");
            GEvent->OnEngineTerminate.Broadcast();

            GAudio      ->Terminate();
            GRender     ->Terminate();
            GAssetHub   ->Terminate();
            GEvent      ->Terminate();

            GRuntime    ->Terminate();

            Status = EStatus::Terminated;
        }

        FEngine() : IGlobalSingleton("Engine") {};
    };

    export inline VISERA_ENGINE_API TUniquePtr<FEngine>
    GEngine = MakeUnique<FEngine>();
}
