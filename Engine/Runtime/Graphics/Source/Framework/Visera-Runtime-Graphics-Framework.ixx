module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics.Framework;
#define VISERA_MODULE_NAME "Runtime.Graphics"
import Visera.Runtime.RHI;
import Visera.Runtime.Graphics.Scene;
import Visera.Runtime.Graphics.PipelineCache;
import Visera.Core.Containers.Array;
import Visera.Core.Types.Pointer;

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

   struct VISERA_RUNTIME_API FRenderBatch
   {
      // Batch key: Pipeline + Material + Mesh
      FRHIRenderPassID      Pipeline;
      FRHIDescriptorSetID   MaterialDescriptorSet;   // Set 0: material textures/samplers
      TSharedPtr<FMesh>     Mesh;                    // nullptr = sprite quad path

      TArray<FInstanceData>  Instances;               // CPU-side per-instance data

      // GPU resources populated by UploadInstanceBuffers (before draw submission)
      FRHIBufferID          InstanceBuffer;           // Storage buffer holding FInstanceData[]
      FRHIDescriptorSetID   InstanceDescriptorSet;    // Set 1: binds InstanceBuffer
   };

   /** List of render batches for each pass type (Sorted). */
   struct VISERA_RUNTIME_API FRenderList
   {
      TArray<FRenderBatch> OpaqueBatches;
      TArray<FRenderBatch> TransparentBatches;
      TArray<FRenderBatch> WireframeBatches;
   };

   /** Per-frame context passed to registered pass factories. Draw passes use RenderList only (no raw Data). Setup passes use RenderWidth/RenderHeight. */
   struct VISERA_RUNTIME_API FRenderContext
   {
      const FRenderList*    RenderList     {nullptr};
      const FRenderView*    RenderView     {nullptr};
      FRHI*                 RHI            {nullptr};
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
