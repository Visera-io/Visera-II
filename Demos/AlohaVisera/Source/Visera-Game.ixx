module;
#include <Visera-Game.hpp>
#include <entt/entt.hpp>
export module Visera.Game;
#define VISERA_MODULE_NAME "Game"
export import Visera.Game.AssetHub;
export import Visera.Audio;
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
    export class VISERA_ENGINE_API FEngine : public IGlobalService
    {
    public:
        TUnicastDelegate<void(Float)>
        AppTick;

        void Run()
        {
            IGlobalService::Bootstrap();

            auto GRHI       = IGlobalService::Get<FRHI>(EName::RHI);
            auto GWindow    = IGlobalService::Get<FWindow>(EName::Window);
            auto GAudio     = IGlobalService::Get<FAudio>(EName::Audio);

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

                    // Logic
                    AppTick.Invoke(DeltaTime);
                    //GEvent ->OnFrameEnd.Broadcast();

                    GRHI->EndFrame();
                    GRHI->Present();
                }
            }
        }
    private:
        FHiResClock Timer;

    public:
        FEngine() : IGlobalService(FName{"Engine"})
        {
            Dependencies =
            {

            };

            if (!OnBootstrap.TryBind([this]
            {
                return True;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        };
    };
}
