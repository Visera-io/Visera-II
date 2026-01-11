module;
#include <../Global/Visera-Game.hpp>
#include <entt/entt.hpp>
export module Visera.Game;
#define VISERA_MODULE_NAME "Game"
export import Visera.Game.AssetHub;
export import Visera.Game.Audio;
export import Visera.Game.Render;
export import Visera.Game.Event;
export import Visera.Game.World;
export import Visera.Global;
       import Visera.Core.Delegate.Unicast;
       import Visera.Core.OS.Time;
       import Visera.Platform;
       import Visera.RHI;
       import Visera.Graphics;
       import Visera.Shader;
       import Visera.Assets;

namespace Visera
{
    export class VISERA_ENGINE_API FEngine : public IGlobalService<FEngine>
    {
    public:
        TUnicastDelegate<void(Float)>
        AppTick;

        void Run()
        {
            VISERA_ASSERT(IsBootstrapped());

            // if (auto Tick = GScripting->GetFunction(PLATFORM_STRING("Tick")))
            // {
            //     if (!AppTick.TryBind([Tick](Float I_DeltaTime)
            //     {
            //         Tick(&I_DeltaTime, sizeof(Float));
            //     }))
            //     { LOG_FATAL("Failed to bind the AppTick()!"); }
            // }
            // else LOG_FATAL("Failed to load the \"AppTick()\" from .NET runtime!");

            if (!GWindow->IsBootstrapped())
            {
                LOG_INFO("Visera Off-Screen Mode.");
                if (GRHI->BeginFrame())
                {
                    //GEvent->OnFrameBegin.Broadcast();
                    AppTick.Invoke(0);
                    //GEvent->OnFrameEnd.Broadcast();
                    GRHI->EndFrame();
                }
                return;
            }

            while (!GWindow->ShouldClose())
            {
                GWindow->PollEvents();
                GAudio->Tick();

                Float DeltaTime = Timer.Tick().Microseconds() / 1000000.0; Timer.Reset();

                if (GRHI->BeginFrame())
                {
                    //GEvent ->OnFrameBegin.Broadcast();

                    if (auto Window = Graphics::GDebug->UI->Window("Hello"))
                    {
                        static Float k = 0;
                        Graphics::GDebug->UI->Slider("K", &k, 0, 1);
                    }

                    // Logic
                    AppTick.Invoke(DeltaTime);
                    GWorld ->Tick(DeltaTime);
                    // Render
                    GRender->Tick(DeltaTime);

                    //GEvent ->OnFrameEnd.Broadcast();

                    GRHI->EndFrame();
                    GRHI->Present();
                }
            }
        }
    private:
        FHiResClock Timer;

    public:
        FEngine() : IGlobalService(FName{"Engine"}) {};
        /*void Bootstrap() override
        {
            LOG_TRACE("Bootstrapping Engine.");
            GRuntime    ->Bootstrap();
            GPlatform   ->Bootstrap();
            GAssets     ->Bootstrap();
            GRHI        ->Bootstrap();
            Graphics::GDebug->Bootstrap(); //[TODO]: Remove
            GShader     ->Bootstrap();

            GEvent      ->Bootstrap();
            GAssetHub   ->Bootstrap();
            GRender     ->Bootstrap();
            GAudio      ->Bootstrap();

            GEvent->OnEngineBootstrap.Broadcast();

            Status = EStatus::Bootstrapped;
        }*/


        /*void Terminate() override
        {
            LOG_TRACE("Terminating Engine.");
            GEvent->OnEngineTerminate.Broadcast();

            GAudio      ->Terminate();
            GRender     ->Terminate();
            GAssetHub   ->Terminate();
            GEvent      ->Terminate();

            GShader     ->Terminate();
            Graphics::GDebug->Terminate(); //[TODO]: Remove
            GRHI        ->Terminate();
            GAssets     ->Terminate();
            GPlatform   ->Terminate();
            GRuntime    ->Terminate();

            Status = EStatus::Terminated;
        }*/
    };

    export inline VISERA_ENGINE_API TUniquePtr<FEngine>
    GEngine = MakeUnique<FEngine>();
}
