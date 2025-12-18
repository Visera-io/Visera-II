module;
#include <Visera-Game.hpp>
export module Visera.Game.Render.Renderer.Interface;
#define VISERA_MODULE_NAME "Game.Render"
export import Visera.RHI;

namespace Visera
{
    export class VISERA_ENGINE_API IRenderer
    {
    public:
        virtual void
        Render(Float I_DeltaTime) = 0;

    protected:
        TSharedPtr<FRHIRenderPipeline>    RenderPipeline;

    public:
        virtual ~IRenderer()
        {
            RenderPipeline.reset();
        }
    };
}
