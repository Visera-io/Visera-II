module;
#include <Visera-Engine.hpp>
export module Visera.Engine.Render;
#define VISERA_MODULE_NAME "Engine.Render"
import Visera.Engine.Render.Sprite;
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_ENGINE_API FRender : public IGlobalSingleton<FRender>
    {
    public:
        void inline
        Tick(Float I_DeltaTime)
        {
            SpriteRenderer->Render();
        }

    private:
        TUniquePtr<FSpriteRenderer> SpriteRenderer;

    public:
        void
        Bootstrap() override
        {
            LOG_TRACE("Bootstrapping Render.");
            SpriteRenderer = MakeUnique<FSpriteRenderer>();
        }
        void
        Terminate() override
        {
            LOG_TRACE("Terminating Render.");
            SpriteRenderer.reset();
        }

        FRender() : IGlobalSingleton("Render") {}
    };

    export inline VISERA_ENGINE_API TUniquePtr<FRender>
    GRender = MakeUnique<FRender>();
}
