module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Graphics.Framework;
#define VISERA_MODULE_NAME "Runtime.Graphics"
import Visera.Runtime.RHI;
import Visera.Runtime.Graphics.Scene.Renderable;
import Visera.Runtime.Graphics.Scene.Light;
import Visera.Runtime.Graphics.Material;
import Visera.Runtime.Graphics.PipelineCache;
import Visera.Core.Containers.Array;
import Visera.Core.Types.Pointer;
import Visera.Core.Math.Algebra;

export namespace Visera
{
   // ── Descriptor set slot convention (matches shader layout) ──
   // Set 0 = per-frame engine data, Set 1 = material textures,
   // Set 2 = lights, Set 3 = per-batch instance data.
   // Binding 0 in each set is engine-managed; binding 1+ reserved for user data.
   inline constexpr UInt32 kFrameDataDescriptorSet     = 0;
   inline constexpr UInt32 kMaterialDescriptorSet      = 1;
   inline constexpr UInt32 kLightDataDescriptorSet     = 2;
   inline constexpr UInt32 kInstanceDataDescriptorSet  = 3;

   // Engine push constants (must match ViseraPushConstants in Core/PushConstants.slang)
   struct FViseraPushConstants
   {
      UInt32 InstanceOffset {0};
   };

   /** App-provided per-frame data passed to shaders via _Frame.UserData0..UserData3. */
   struct VISERA_RUNTIME_API FUserFrameData
   {
      FVector4F Data0;
      FVector4F Data1;
      FVector4F Data2;
      FVector4F Data3;
   };

   /** Engine per-frame UBO (std140, bound at Set 0 Binding 0).
    *  Must match EngineFrameData in FrameData.slang. */
   struct VISERA_RUNTIME_API FEngineFrameData
   {
      FMatrix4x4F ViewMatrix;
      FMatrix4x4F ProjectionMatrix;
      FMatrix4x4F ViewProjectionMatrix;
      FVector4F   RenderSize;      // xy = width/height, zw = 1/width, 1/height
      FUserFrameData UserFrameData;
   };

   /** GPU-side light (std430, bound at Set 2 Binding 0).
    *  Must match GPULight in LightData.slang. */
   struct VISERA_RUNTIME_API FGPULight
   {
      UInt32    Type;         // 0=Directional, 1=Point, 2=Spot
      FVector3F Position;
      FVector3F Direction;
      Float     Range;
      FVector4F Color;        // rgb + alpha (unused)
      Float     Intensity;
      Float     SpotAngle;
      UInt32    bCastShadow;
      Float     _Padding {0.f};
   };

   /** Per-frame scene payload: draw commands and lights. Read-only on the graphics thread. */
   struct VISERA_RUNTIME_API FRenderData
   {
      TArray<FRenderableMeta> DrawCommands;
      TArray<FLight>          Lights;

      [[nodiscard]] const TArray<FRenderableMeta>&
      GetDrawCommands() const { return DrawCommands; }
      [[nodiscard]] const TArray<FLight>&
      GetLights() const { return Lights; }
   };

   /** Per-frame view payload: view and projection matrices. Built from FCamera. */
   struct VISERA_RUNTIME_API FRenderView
   {
      FMatrix4x4F ViewMatrix       {FMatrix4x4F::Identity()};
      FMatrix4x4F ProjectionMatrix {FMatrix4x4F::Identity()};
   };

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
      FUserFrameData  UserFrameData;
   };

   struct VISERA_RUNTIME_API FRenderBatch
   {
      const FMaterial*       Material;                // Deferred pipeline resolution (pass decides format)
      FRHIDescriptorSetID    MaterialDescriptorSet;   // Set 1: material textures/samplers
      TSharedPtr<FMesh>      Mesh;                    // nullptr = sprite quad path

      TArray<FInstanceData>  Instances;               // CPU-side per-instance data

      // GPU resources populated by UploadInstanceBuffers (before draw submission)
      FRHIDescriptorSetID    InstanceDescriptorSet;   // Set 3: shared page or overflow instance buffer
      UInt32                 InstanceStartIndex {0};  // Instance index offset inside the bound instance buffer page
   };

   /** List of render batches for each pass type (sorted by material/mesh). */
   struct VISERA_RUNTIME_API FRenderList
   {
      TArray<FRenderBatch> OpaqueBatches;
      TArray<FRenderBatch> TransparentBatches;
      TArray<FRenderBatch> WireframeBatches;
   };

   /** Per-frame context bridging the graphics thread and RenderGraph execution. */
   struct VISERA_RUNTIME_API FRenderContext
   {
      const FRenderList*    RenderList              {nullptr};
      const FRenderView*    RenderView              {nullptr};
      FRHI*                 RHI                     {nullptr};
      FRHISwapChainID       SwapChainID             {kInvalidSwapChainID};
      FRHITextureID         BackBuffer;
      FPipelineCache*       PipelineCache           {nullptr};
      FRHIBufferID          FrameDataUBO;
      FRHIDescriptorSetID   FrameDataDescriptorSet;  // Set 0
      FRHIBufferID          LightSSBO;
      FRHIDescriptorSetID   LightDescriptorSet;      // Set 2
      UInt32                RenderWidth             {0};
      UInt32                RenderHeight            {0};
   };

   struct VISERA_RUNTIME_API ERenderPassPriority
   {
      static constexpr UInt32
      ShadowMap      = 500;
      static constexpr UInt32
      Opaque         = 1000;
      static constexpr UInt32
      OpaqueSprites  = 2000;
      static constexpr UInt32
      Transparent    = 3000;
      static constexpr UInt32
      PostProcess    = 5000;
      static constexpr UInt32
      FinalBlit      = 10000;
   };
}
