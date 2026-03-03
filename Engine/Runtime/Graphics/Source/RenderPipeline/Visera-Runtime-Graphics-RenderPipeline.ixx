module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics.RenderPipeline;
#define VISERA_MODULE_NAME "Runtime.Graphics"
export import Visera.Runtime.Graphics.RenderPipeline.RenderGraph;
       import Visera.Runtime.RHI;
       import Visera.Runtime.Graphics.Scene;

export namespace Visera
{
    /** Per-frame context passed to registered pass factories. Contains RHI, scene snapshot, swap chain, and back buffer for the current frame. */
    struct VISERA_RUNTIME_API FRenderContext
    {
        FRHI*                    RHI         {nullptr};
        const FScene::FSnapshot* Scene       {nullptr};
        FRHISwapChainID          SwapChainID {kInvalidSwapChainID};
        FRHITextureID            BackBuffer  {};
    };
}
