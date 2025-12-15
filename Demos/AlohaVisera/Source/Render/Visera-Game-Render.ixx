module;
#include <Visera-Game.hpp>
export module Visera.Game.Render;
#define VISERA_MODULE_NAME "Game.Render"
import Visera.Game.Render.Background;
import Visera.Game.Render.Sprite;
import Visera.Game.Render.Cube;
import Visera.Game.Render.Line;
import Visera.Core.Global;
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_ENGINE_API FRender : public IGlobalSingleton<FRender>
    {
    public:
        void inline
        Tick(Float I_DeltaTime)
        {
            //BackgroundRenderer->Render(I_DeltaTime);
            SpriteRenderer->Render(I_DeltaTime);
            //LineRenderer->Render(I_DeltaTime);
            CubeRenderer->Render(I_DeltaTime);
        }

    private:
        TUniquePtr<FBackgroundRenderer> BackgroundRenderer;
        TUniquePtr<FSpriteRenderer>     SpriteRenderer;
        TUniquePtr<FLineRenderer>       LineRenderer;
        TUniquePtr<FCubeRenderer>       CubeRenderer;

    public:
        void
        Bootstrap() override
        {
            LOG_TRACE("Bootstrapping Render.");
            BackgroundRenderer = MakeUnique<FBackgroundRenderer>();
            SpriteRenderer = MakeUnique<FSpriteRenderer>();
            LineRenderer = MakeUnique<FLineRenderer>();
            CubeRenderer = MakeUnique<FCubeRenderer>();
        }
        void
        Terminate() override
        {
            LOG_TRACE("Terminating Render.");
            CubeRenderer.reset();
            LineRenderer.reset();
            SpriteRenderer.reset();
            BackgroundRenderer.reset();
        }

        FRender() : IGlobalSingleton("Render") {}
    };

    export inline VISERA_ENGINE_API TUniquePtr<FRender>
    GRender = MakeUnique<FRender>();
}
