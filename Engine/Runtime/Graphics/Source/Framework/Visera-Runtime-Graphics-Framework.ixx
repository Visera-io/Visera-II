module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics.Framework;
#define VISERA_MODULE_NAME "Runtime.Graphics"
import Visera.Runtime.RHI;
import Visera.Runtime.Graphics.Scene;
import Visera.Runtime.Graphics.PipelineCache;

export namespace Visera
{
   struct VISERA_RUNTIME_API FRenderArea
   {
      UInt32 Width  {0}; // 0 -> SwapChain size
      UInt32 Height {0}; // 0 -> SwapChain size
   };

   struct VISERA_RUNTIME_API FRenderTask
   {
      FRHISwapChainID SwapChainID {kInvalidSwapChainID};
      FRenderData     Data;
      FRenderView     RenderView;
      FRenderArea     RenderArea;
   };

   /** Per-frame context passed to registered pass factories. Contains RHI, render data and view, swap chain, and back buffer for the current frame. */
   struct VISERA_RUNTIME_API FRenderContext
   {
      FRHI*                 RHI            {nullptr};
      const FRenderData*    Data           {nullptr};
      const FRenderView*    RenderView     {nullptr};
      FRHISwapChainID       SwapChainID    {kInvalidSwapChainID};
      FRHITextureID         BackBuffer;
      FPipelineCache*       PipelineCache  {nullptr};
      UInt32                RenderWidth    {0};
      UInt32                RenderHeight   {0};
   };

   struct VISERA_RUNTIME_API ERenderPassPriority
   {
      static constexpr UInt32
      Setup          = 100;
      static constexpr UInt32
      Opaque         = 1000;
      static constexpr UInt32
      OpaqueSprites  = 2000;  // RenderArea.Width/Height is the internal resolution for scene passes.
      static constexpr UInt32
      FinalBlit      = 10000; // FinalBlit is the last pass to be executed.
   };
}
