module;
#include <Visera-Engine.hpp>
export module Visera.Engine.Render.Renderer;
#define VISERA_MODULE_NAME "Engine.Render"
export import Visera.Runtime.RHI;

namespace Visera
{
    export class VISERA_ENGINE_API IRenderer
    {
    public:
        virtual void
        Render() const = 0;

    protected:
        TSharedPtr<RHI::FRenderPass>    RenderPass;

    public:
        virtual ~IRenderer()
        {
            RenderPass.reset();
        }
    };
}
