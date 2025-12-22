module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics.Renderer.Postprocess;
#define VISERA_MODULE_NAME "Graphics.Renderer"
import Visera.Graphics.RenderPass.Tonemap;

export namespace Visera
{
    class VISERA_GRAPHICS_API FPostprocessRenderer
    {
    public:

    private:
        TUniquePtr<FTonemapRenderPass> TonemapPass;
    };
}