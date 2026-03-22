module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Graphics;
#define VISERA_MODULE_NAME "Runtime.Graphics"
export import Visera.Runtime.Graphics.Scene;
export import Visera.Runtime.Graphics.Framework;
export import Visera.Runtime.Graphics.Material;
export import Visera.Runtime.Graphics.PipelineCache;
export import Visera.Runtime.Graphics.RenderGraph;
export import Visera.Runtime.Graphics.RenderPass;
       import Visera.Runtime.AssetHub;
       import Visera.Runtime.RHI;
       import Visera.Runtime.Window;
       import Visera.Core.Containers.Array;
       import Visera.Core.Containers.Map;
       import Visera.Core.Math.Hash.GoldenRatio;
       import Visera.Core.Concurrency.Channel.SPSC;
       import Visera.Core.OS.Thread;
       import Visera.Core.OS.Time;
       import Visera.Core.Types.Function;
       import Visera.Core.Types.Optional;
       import Visera.Core.Types.Pointer;
       import Visera.Core.Types.Path;
       import Visera.Core.Types.JSON;
       import Visera.Core.Types.Name;
       import Visera.Core.Log;

/** Pre-created FName constant; avoids per-frame construction in the graphics thread hot path. */
export namespace Visera::EName
{
   inline const FName PresentTransition {"PresentTransition"};
}

export namespace Visera
{
   /** A registered pass entry. Graphics exclusively owns the IRenderPass lifetime. */
   struct VISERA_RUNTIME_API FRenderPass
   {
      UInt32                  Priority {0};
      FName                   Name;
      TUniquePtr<IRenderPass> Pass;
   };

   /** Hash (Material*, Mesh*) pair into a single UInt64 for batch-key dedup. */
   [[nodiscard]] inline UInt64
   MakeBatchKey(const FMaterial* I_Mat, const FMesh* I_Mesh) noexcept
   {
      return Math::GoldenRatioHashCombine(
         reinterpret_cast<UInt64>(I_Mat),
         reinterpret_cast<UInt64>(I_Mesh));
   }

   struct FInstanceDataPage
   {
      FRHIBufferID Buffer;
      FRHIDescriptorSetID DescriptorSet;
   };

   struct FFrameSlotFrameData
   {
      FRHIBufferID        Buffer;
      FRHIDescriptorSetID DescriptorSet;
   };

   struct FFrameSlotLightBuffer
   {
      FRHIBufferID        Buffer;
      FRHIDescriptorSetID DescriptorSet;
      UInt64              CurrentCapacity {0};
   };

   struct FInstanceAllocatorState
   {
      UInt32 CurrentPageIndex {0};
      UInt64 CurrentOffsetInPage {0};
   };

   struct FFrameSlotInstanceState
   {
      TArray<FInstanceDataPage> InstanceDataPages;
      FInstanceAllocatorState   AllocatorState;
      FRHIBufferID              OverflowInstanceBuffer;
      FRHIDescriptorSetID       OverflowInstanceDescriptorSet;
      UInt64                    OverflowOffsetInBuffer {0};
      UInt64                    OverflowBufferByteSize {0};

      void
      ResetForFrame()
      {
         AllocatorState = {};
         OverflowInstanceBuffer = {};
         OverflowInstanceDescriptorSet = {};
         OverflowOffsetInBuffer = 0;
         OverflowBufferByteSize = 0;
      }
   };

   [[nodiscard]] inline UInt64
   AlignUpTo(UInt64 I_Value, UInt64 I_Alignment)
   {
      if (I_Alignment == 0) { return I_Value; }
      return ((I_Value + I_Alignment - 1) / I_Alignment) * I_Alignment;
   }

   /** CreateInfo for FGraphics. RHI and AssetHub are required; they are passed as non-owning pointers. */
   struct VISERA_RUNTIME_API FGraphicsCreateInfo
   {
      FRHI*      RHI      = nullptr;
      FAssetHub* AssetHub = nullptr;
   };

   [[nodiscard]] inline FRHIDescriptorSetID
   CreateInstanceDescriptorSet(FRHI* I_RHI, const FRHIBufferID& I_Buffer)
   {
      FRHIDescriptorSetID DescriptorSet = I_RHI->CreateDescriptorSet(FRHIDescriptorSetCreateInfo{
         .Bindings = {{
            .Binding = 0,
            .Type    = ERHIDescriptorType::StorageBuffer,
            .Count   = 1,
            .Stages  = ERHIShaderStage::Vertex | ERHIShaderStage::Fragment,
         }},
      });
      I_RHI->WriteDescriptorStorageBuffer(DescriptorSet, 0, I_Buffer);
      return DescriptorSet;
   }

   [[nodiscard]] inline FFrameSlotFrameData
   CreateFrameDataUBO(FRHI* I_RHI)
   {
      FRHIBufferID Buffer = I_RHI->CreateBuffer(FRHIBufferCreateInfo{
         .Size          = sizeof(FEngineFrameData),
         .Usages        = ERHIBufferUsage::UniformBuffer,
         .bHostWritable = True,
      });
      FRHIDescriptorSetID DescriptorSet = I_RHI->CreateDescriptorSet(FRHIDescriptorSetCreateInfo{
         .Bindings = {{
            .Binding = 0,
            .Type    = ERHIDescriptorType::UniformBuffer,
            .Count   = 1,
            .Stages  = ERHIShaderStage::Vertex | ERHIShaderStage::Fragment,
         }},
      });
      I_RHI->WriteDescriptorUniformBuffer(DescriptorSet, 0, Buffer);
      return FFrameSlotFrameData{ .Buffer = Buffer, .DescriptorSet = DescriptorSet };
   }

   [[nodiscard]] inline FRHIDescriptorSetID
   CreateLightDescriptorSet(FRHI* I_RHI, const FRHIBufferID& I_Buffer)
   {
      FRHIDescriptorSetID DescriptorSet = I_RHI->CreateDescriptorSet(FRHIDescriptorSetCreateInfo{
         .Bindings = {{
            .Binding = 0,
            .Type    = ERHIDescriptorType::StorageBuffer,
            .Count   = 1,
            .Stages  = ERHIShaderStage::Vertex | ERHIShaderStage::Fragment,
         }},
      });
      I_RHI->WriteDescriptorStorageBuffer(DescriptorSet, 0, I_Buffer);
      return DescriptorSet;
   }

   [[nodiscard]] inline FFrameSlotLightBuffer
   CreateLightSSBO(FRHI* I_RHI)
   {
      static constexpr UInt64 kInitialLightCapacity = 8;
      FRHIBufferID Buffer = I_RHI->CreateBuffer(FRHIBufferCreateInfo{
         .Size          = kInitialLightCapacity * sizeof(FGPULight),
         .Usages        = ERHIBufferUsage::StorageBuffer,
         .bHostWritable = True,
      });
      FRHIDescriptorSetID DescriptorSet = CreateLightDescriptorSet(I_RHI, Buffer);
      return FFrameSlotLightBuffer{ .Buffer = Buffer, .DescriptorSet = DescriptorSet, .CurrentCapacity = kInitialLightCapacity };
   }

   inline void
   EnsureInstanceDataPages(FFrameSlotInstanceState& IO_FrameSlotState, FRHI* I_RHI)
   {
      if (!IO_FrameSlotState.InstanceDataPages.IsEmpty()) { return; }

      IO_FrameSlotState.InstanceDataPages.Resize(kInstanceDataPageCount);
      for (auto& Page : IO_FrameSlotState.InstanceDataPages)
      {
         Page.Buffer = I_RHI->CreateBuffer(FRHIBufferCreateInfo{
            .Size          = kInstanceDataPageSize,
            .Usages        = ERHIBufferUsage::StorageBuffer,
            .bHostWritable = True,
         });
         Page.DescriptorSet = CreateInstanceDescriptorSet(I_RHI, Page.Buffer);
      }
   }

   [[nodiscard]] inline Bool
   TryAllocateFromPages(FFrameSlotInstanceState& IO_FrameSlotState,
                        UInt64 I_AlignedByteSize,
                        UInt32& O_PageIndex,
                        UInt64& O_OffsetInPage)
   {
      if (I_AlignedByteSize > kInstanceDataPageSize) { return False; }

      auto AllocatorState = IO_FrameSlotState.AllocatorState;
      while (AllocatorState.CurrentPageIndex < IO_FrameSlotState.InstanceDataPages.GetSize())
      {
         if (AllocatorState.CurrentOffsetInPage + I_AlignedByteSize <= kInstanceDataPageSize)
         {
            O_PageIndex = AllocatorState.CurrentPageIndex;
            O_OffsetInPage = AllocatorState.CurrentOffsetInPage;
            AllocatorState.CurrentOffsetInPage += I_AlignedByteSize;
            IO_FrameSlotState.AllocatorState = AllocatorState;
            return True;
         }

         AllocatorState.CurrentPageIndex += 1;
         AllocatorState.CurrentOffsetInPage = 0;
      }

      return False;
   }

   /** Fills O_List by iterating I_Data draw commands, batching by (Material, Mesh). Routes by surface (Opaque/Transparent).
    *  Pipeline resolution is deferred to DrawBatch (pass decides target format). */
   inline void
   BatchAndSort(const FRenderData& I_Data, FRenderList& O_List)
   {
      O_List.OpaqueBatches.Clear();
      O_List.TransparentBatches.Clear();
      O_List.WireframeBatches.Clear();
      TMap<UInt64, UInt32> OpaqueMap;
      TMap<UInt64, UInt32> TransparentMap;
      const auto& DrawCommands = I_Data.GetDrawCommands();
      for (const auto& DrawCommand : DrawCommands)
      {
         const FMaterial* Mat = DrawCommand.Material.Get();
         if (!Mat || !Mat->IsValid()) { continue; }
         const FInstanceData& InstanceData = DrawCommand.InstanceData;
         const auto& MeshPtr = DrawCommand.Mesh;
         const UInt64 Key = MakeBatchKey(Mat, MeshPtr.Get());
         const ESurfaceType Surface = Mat->GetSurface();
         const Bool bTransparent = (Surface == ESurfaceType::Transparent);
         TArray<FRenderBatch>& FillBatches = bTransparent ? O_List.TransparentBatches : O_List.OpaqueBatches;
         TMap<UInt64, UInt32>& FillMap = bTransparent ? TransparentMap : OpaqueMap;
         auto FillIt = FillMap.Find(Key);
         if (FillIt == FillMap.end())
         {
            FRenderBatch Batch {
               .Material = Mat,
               .MaterialDescriptorSet = Mat->GetDescriptorSet(),
               .Mesh = MeshPtr,
               .Instances = {},
            };
            Batch.Instances.PushBack(InstanceData);
            const UInt32 Idx = FillBatches.GetSize();
            FillBatches.PushBack(std::move(Batch));
            FillMap.Insert(Key, Idx);
         }
         else
         { FillBatches[FillIt->second].Instances.PushBack(InstanceData); }
      }
   }

   /** Upload batched instance data into per-frame-slot shared pages.
    *  Pages are persistent across frames; overflow is scoped to the reused frame slot. */
   inline void
   UploadInstanceBuffers(FRenderList& IO_List,
                         FRHI* I_RHI,
                         FFrameSlotInstanceState& IO_FrameSlotState)
   {
      IO_FrameSlotState.ResetForFrame();
      EnsureInstanceDataPages(IO_FrameSlotState, I_RHI);

      struct FBatchUploadWork
      {
         FRenderBatch* Batch {nullptr};
         UInt64        ByteSize {0};
         UInt64        AlignedByteSize {0};
      };

      TArray<FBatchUploadWork> UploadWorkItems;
      auto CollectUploadWork = [&UploadWorkItems](TArray<FRenderBatch>& I_Batches)
      {
         for (auto& Batch : I_Batches)
         {
            if (Batch.Instances.IsEmpty()) { continue; }

            const UInt64 ByteSize = Batch.Instances.GetSize() * sizeof(FInstanceData);
            UploadWorkItems.PushBack(FBatchUploadWork{
               .Batch = &Batch,
               .ByteSize = ByteSize,
               .AlignedByteSize = AlignUpTo(ByteSize, kInstanceDataBufferAlignment),
            });
         }
      };
      CollectUploadWork(IO_List.OpaqueBatches);
      CollectUploadWork(IO_List.TransparentBatches);

      FFrameSlotInstanceState SimulatedFrameSlotState;
      SimulatedFrameSlotState.InstanceDataPages.Resize(IO_FrameSlotState.InstanceDataPages.GetSize());
      UInt64 RequiredOverflowByteSize = 0;
      for (const auto& UploadWorkItem : UploadWorkItems)
      {
         UInt32 SimulatedPageIndex = 0;
         UInt64 SimulatedOffsetInPage = 0;
         if (!TryAllocateFromPages(
               SimulatedFrameSlotState,
               UploadWorkItem.AlignedByteSize,
               SimulatedPageIndex,
               SimulatedOffsetInPage))
         {
            RequiredOverflowByteSize += UploadWorkItem.AlignedByteSize;
         }
      }

      for (const auto& UploadWorkItem : UploadWorkItems)
      {
         UInt32 PageIndex = 0;
         UInt64 OffsetInPage = 0;
         if (TryAllocateFromPages(IO_FrameSlotState, UploadWorkItem.AlignedByteSize, PageIndex, OffsetInPage))
         {
            auto& Page = IO_FrameSlotState.InstanceDataPages[PageIndex];
            I_RHI->WriteBufferDirect(
               Page.Buffer,
               reinterpret_cast<const FByte*>(UploadWorkItem.Batch->Instances.Data()),
               UploadWorkItem.ByteSize,
               OffsetInPage);
            UploadWorkItem.Batch->InstanceDescriptorSet = Page.DescriptorSet;
            UploadWorkItem.Batch->InstanceStartIndex = static_cast<UInt32>(OffsetInPage / sizeof(FInstanceData));
            continue;
         }

         if (IO_FrameSlotState.OverflowInstanceBuffer.IsNull())
         {
            UInt64 OverflowBufferByteSize = RequiredOverflowByteSize;
            if (OverflowBufferByteSize < UploadWorkItem.AlignedByteSize)
            {
               OverflowBufferByteSize = UploadWorkItem.AlignedByteSize;
            }
            OverflowBufferByteSize = AlignUpTo(OverflowBufferByteSize, kInstanceDataBufferAlignment);

            LOG_WARN("Instance data exceeded the pre-allocated page budget; using a temporary overflow buffer for this frame slot.");
            IO_FrameSlotState.OverflowInstanceBuffer = I_RHI->CreateBuffer(FRHIBufferCreateInfo{
               .Size          = OverflowBufferByteSize,
               .Usages        = ERHIBufferUsage::StorageBuffer,
               .bHostWritable = True,
            });
            IO_FrameSlotState.OverflowInstanceDescriptorSet =
               CreateInstanceDescriptorSet(I_RHI, IO_FrameSlotState.OverflowInstanceBuffer);
            IO_FrameSlotState.OverflowBufferByteSize = OverflowBufferByteSize;
         }

         VISERA_ASSERT(IO_FrameSlotState.OverflowOffsetInBuffer + UploadWorkItem.AlignedByteSize <= IO_FrameSlotState.OverflowBufferByteSize);
         const UInt64 OverflowOffset = IO_FrameSlotState.OverflowOffsetInBuffer;
         I_RHI->WriteBufferDirect(
            IO_FrameSlotState.OverflowInstanceBuffer,
            reinterpret_cast<const FByte*>(UploadWorkItem.Batch->Instances.Data()),
            UploadWorkItem.ByteSize,
            OverflowOffset);
         UploadWorkItem.Batch->InstanceDescriptorSet = IO_FrameSlotState.OverflowInstanceDescriptorSet;
         UploadWorkItem.Batch->InstanceStartIndex = static_cast<UInt32>(OverflowOffset / sizeof(FInstanceData));
         IO_FrameSlotState.OverflowOffsetInBuffer += UploadWorkItem.AlignedByteSize;
      }
   }

   class VISERA_RUNTIME_API FGraphics
   {
   public:
      explicit FGraphics(const FGraphicsCreateInfo& I_CreateInfo);
      ~FGraphics();

   public:
      /** Per-frame camera (cleared after Render). */
      void
      SetCamera(FCamera I_Camera);

      /** Per-frame light submission (cleared after Render). */
      void
      SubmitLight(FLight I_Light);

      /** Per-frame draw submission (cleared after Render). Extracts data via ToRenderableMeta(). */
      void
      Draw(const IRenderable& I_Renderable);

      /** Per-frame draw submission (cleared after Render). Direct meta; used by scripting bindings. */
      void
      Draw(const FRenderableMeta& I_Meta);

      /** Build FRenderTask from accumulated draws/camera/lights and send to graphics thread; clear per-frame state. Called by engine after OnPreRender. */
      void
      Render(FWindow* I_Window);

      /** Create a headless rendering context. Returns a stable SwapChainID. */
      [[nodiscard]] FRHISwapChainID
      RegisterHeadless();

      /** Unregister a window and destroy its swap chain. Call before destroying the FWindow (e.g. before app terminate) to avoid dangling pointers. */
      void
      UnregisterWindow(FWindow* I_Window);

      /** Declare a persistent named texture managed by the Graphics layer.
       *  Width/Height == 0 means auto-match the render area each frame. */
      void
      CreateNamedTexture(FName I_Name, const FRDGTextureCreateInfo& I_CreateInfo);

      /** Register a class-based render pass. Graphics takes exclusive ownership. Lower priority values execute first. */
      void
      RegisterPass(UInt32 I_Priority, FName I_Name, TUniquePtr<IRenderPass> I_Pass);

      /** Register a lambda-based render pass (convenience overload). */
      void
      RegisterPass(UInt32 I_Priority, FName I_Name,
                   TFunction<void(FRDGPassBuilder&)> I_Setup,
                   TFunction<void(FRDGPassContext&)> I_Execute);

      /** Remove a previously registered pass by name. Safe to call from the main thread. */
      void
      UnregisterPass(FName I_Name);

      [[nodiscard]] TSharedPtr<FMaterial>
      LoadMaterial(const VPath& I_MaterialPath);

      /** Set app-specific user data packed into FrameData (shader reads via _Frame.UserData0..UserData3). */
      void
      SetUserFrameData(const FUserFrameData& I_Data)
      { UserFrameData.Store(I_Data, EMemoryOrder::Relaxed); }

      [[nodiscard]] const FRHI*
      GetRHI() const { return RHI; }

      /** Frame rate (FPS) for I_Window from RHI (swap chain present timing). 0 if I_Window is null or no windowed swap chain. */
      [[nodiscard]] Float
      GetFrameRate(FWindow* I_Window) const;

   private:

      /** Persistent named texture declaration stored by CreateNamedTexture. */
      struct FNamedTextureDeclaration
      {
         FName                 Name;
         FRDGTextureCreateInfo Descriptor;
         Bool                  bMatchRenderArea {False};
      };

      /** Cached texture descriptor for cross-frame transient reuse. */
      struct FCachedTexture
      {
         FRHITextureID   ID;
         UInt32          Width       {0};
         UInt32          Height      {0};
         ERHIFormat      Format      {ERHIFormat::Undefined};
         ERHIImageLayout KnownLayout {ERHIImageLayout::Undefined};
      };

      /** Per-swapchain retained state for RenderGraph. Each FWindow / headless context owns one. */
      struct FSwapChainGraphContext
      {
         UInt64                                FrameSerial {0};
         TArray<TMap<FName, FCachedTexture>>   InFlightCaches;
         TArray<FFrameSlotInstanceState>       FrameSlotInstanceStates;
         TArray<FFrameSlotFrameData>           FrameSlotFrameData;
         TArray<FFrameSlotLightBuffer>         FrameSlotLightBuffers;
         FPipelineCache                        PipelineCache;

         void Init(UInt32 I_MaxInFlightFrames, FRHI* I_RHI)
         {
            InFlightCaches.Resize(I_MaxInFlightFrames);
            FrameSlotInstanceStates.Resize(I_MaxInFlightFrames);
            FrameSlotFrameData.Resize(I_MaxInFlightFrames);
            FrameSlotLightBuffers.Resize(I_MaxInFlightFrames);
            for (auto& Slot : FrameSlotFrameData)
            { Slot = CreateFrameDataUBO(I_RHI); }
            for (auto& Slot : FrameSlotLightBuffers)
            { Slot = CreateLightSSBO(I_RHI); }
         }

         void InvalidateCache()
         {
            for (auto& Slot : InFlightCaches) { Slot.Clear(); }
         }
      };

      /** Dedicated thread: take draw intent from channel -> BeginFrame -> inject named textures -> add IRenderPass nodes -> PresentTransition -> Compile -> Execute -> Present -> EndFrame. */
      struct VISERA_RUNTIME_API FGraphicsThread
      {
      public:
         FGraphicsThread(
            FRHI*                                     I_RHI,
            TSPSCChannel<FRenderTask>&                I_ChannelFromMain,
            TAtomic<UInt32>&                          I_PendingDrawRenderTaskCount,
            FEvent*                                   I_FrameConsumedEvent,
            const TArray<FRenderPass>*                I_RenderPasses,
            FRWLock*                                  I_RenderPassesLock,
            const TArray<FNamedTextureDeclaration>*   I_NamedTextureDeclarations,
            FRWLock*                                  I_NamedTexturesLock,
            UInt32                                    I_MaxFrameRate);

         void
         Start();
         void
         RequestStop();
         void
         Join();

      private:
         void
         Run();

         [[nodiscard]] FSwapChainGraphContext&
         GetOrCreateContext(FRHISwapChainID I_ID);

         void
         WaitSwapChainReadyIfDirty(FRHISwapChainID I_SwapChainID);
         /** Inject declared named textures into I_Graph (reusing cached RHI allocations when available). */
         void
         InjectNamedTextures(FRenderGraph* I_Graph, TMap<FName, FCachedTexture>& I_CacheSlot,
                             UInt32 I_RenderWidth, UInt32 I_RenderHeight);
         /** Persist named textures from I_Graph into I_CacheSlot for future frame reuse. */
         void
         ExportGraphToCache(FRenderGraph& I_Graph, TMap<FName, FCachedTexture>& I_CacheSlot);
         void
         ApplyFramePacing(FHighResTimePoint I_FrameStart);
         void
         CompileAndSubmit(const FRenderContext& RenderContext, FRenderGraph* Graph);

         FRHI*                                 RHI {nullptr};
         TSPSCChannel<FRenderTask>&            ChannelFromMain;
         TAtomic<UInt32>&                      PendingDrawRenderTaskCount;
         FEvent*                               FrameConsumedEvent {nullptr};
         UInt32                                MaxFrameRate {70};
         FHiResClock                           FramePacingClock;
         TUniquePtr<FThread>                   Thread;
         const TArray<FRenderPass>*            RenderPasses {nullptr};
         FRWLock*                              RenderPassesLock {nullptr};
         const TArray<FNamedTextureDeclaration>* NamedTextureDeclarations {nullptr};
         FRWLock*                              NamedTexturesLock {nullptr};

         TArray<FSwapChainGraphContext> SwapChainContexts;
      };

      FAssetHub*                                AssetHub {nullptr};
      FRHI*                                     RHI      {nullptr};
      TSPSCChannel<FRenderTask>                 ChannelToGraphics;
      TAtomic<UInt32>                           PendingDrawRenderTaskCount {0};
      FEvent                                    FrameConsumedEvent;
      TAtomic<Bool>                             bShuttingDown {False};

      TUniquePtr<FGraphicsThread>               GraphicsThread;
      mutable FRWLock                           RenderPassesLock;
      TArray<FRenderPass>                       RenderPasses;
      mutable FRWLock                           NamedTexturesLock;
      TArray<FNamedTextureDeclaration>          NamedTextureDeclarations;

      TInlineArray<FRHISwapChainID, kMaxSwapChainCount> ManagedHeadlessIDs;
      TInlineArray<FWindow*, kMaxSwapChainCount>        ManagedWindows;

      TOptional<FCamera>       FrameCamera;
      TArray<FLight>           FrameLights;
      TArray<FRenderableMeta>  PendingDraws;

      TMap<FName, TSharedPtr<FMaterial>> MaterialCache;

      TAtomic<FUserFrameData>  UserFrameData;
   };

   FGraphics::FGraphics(const FGraphicsCreateInfo& I_CreateInfo)
   {
      if (!I_CreateInfo.RHI || !I_CreateInfo.AssetHub)
      { LOG_FATAL("FGraphicsCreateInfo requires non-null RHI and AssetHub!"); return; }
      RHI = I_CreateInfo.RHI;
      AssetHub = I_CreateInfo.AssetHub;

      GraphicsThread = MakeUnique<FGraphicsThread>(
         RHI, ChannelToGraphics, PendingDrawRenderTaskCount,
         &FrameConsumedEvent, &RenderPasses, &RenderPassesLock,
         &NamedTextureDeclarations, &NamedTexturesLock, 0u);
      GraphicsThread->Start();
      LOG_INFO("Graphics: thread started (frame pacing on main thread).");
   }

   FGraphics::~FGraphics()
   {
      if (GraphicsThread)
      {
         bShuttingDown.Store(True, EMemoryOrder::Relaxed);
         FrameConsumedEvent.Trigger();
         GraphicsThread->RequestStop();
         GraphicsThread->Join();
         GraphicsThread.Reset();
      }
      if (RHI) { RHI->WaitDeviceIdle(); }
      RenderPasses.Clear();
      for (FWindow* W : ManagedWindows)
      { RHI->DestroySwapChain(W); }
      for (auto Id : ManagedHeadlessIDs)
      { RHI->DestroySwapChain(Id); }
      ManagedWindows.Clear();
      ManagedHeadlessIDs.Clear();
      PROFILING_ONLY_FIELD(LogRenderGraphCompileProfilingSummary();)
      LOG_DEBUG("Graphics: terminated.");
   }

   // =================================================================
   // CreateNamedTexture
   // =================================================================

   void FGraphics::
   CreateNamedTexture(FName I_Name, const FRDGTextureCreateInfo& I_CreateInfo)
   {
      FScopeWriteLock Lock(&NamedTexturesLock);
      Bool bMatchRenderArea = (I_CreateInfo.Width == 0 && I_CreateInfo.Height == 0);
      NamedTextureDeclarations.PushBack(FNamedTextureDeclaration{
         .Name              = I_Name,
         .Descriptor        = I_CreateInfo,
         .bMatchRenderArea  = bMatchRenderArea,
      });
      LOG_INFO("Graphics::CreateNamedTexture: '{}' declared (matchRenderArea={}).",
               I_Name.GetNameString(), bMatchRenderArea);
   }

   // =================================================================
   // RegisterPass
   // =================================================================

   void FGraphics::
   RegisterPass(UInt32 I_Priority, FName I_Name, TUniquePtr<IRenderPass> I_Pass)
   {
      FScopeWriteLock Lock(&RenderPassesLock);
      UInt64 InsertIdx = 0;
      for (; InsertIdx < RenderPasses.GetSize(); ++InsertIdx)
      {
         if (RenderPasses[InsertIdx].Priority > I_Priority) { break; }
      }
      RenderPasses.Insert(RenderPasses.begin() + InsertIdx, FRenderPass
      {
         .Priority = I_Priority,
         .Name     = I_Name,
         .Pass     = std::move(I_Pass),
      });
      LOG_INFO("Graphics::RegisterPass: '{}' registered at priority {}.", I_Name.GetNameString(), I_Priority);
   }

   void FGraphics::
   RegisterPass(UInt32 I_Priority, FName I_Name,
                TFunction<void(FRDGPassBuilder&)> I_Setup,
                TFunction<void(FRDGPassContext&)> I_Execute)
   {
      RegisterPass(I_Priority, std::move(I_Name),
                   MakeUnique<FLambdaRenderPass>(std::move(I_Setup), std::move(I_Execute)));
   }

   void FGraphics::
   UnregisterPass(FName I_Name)
   {
      FScopeWriteLock Lock(&RenderPassesLock);
      for (auto It = RenderPasses.begin(); It != RenderPasses.end(); ++It)
      {
         if (It->Name == I_Name)
         {
            LOG_INFO("Graphics::UnregisterPass: '{}' removed.", I_Name.GetNameString());
            RenderPasses.Erase(It);
            return;
         }
      }
      LOG_WARN("Graphics::UnregisterPass: '{}' not found.", I_Name.GetNameString());
   }

   // =================================================================
   // Pipeline helpers (used inside FGraphicsThread::Run)
   // =================================================================

   /** Culling stub -- pass-through until frustum/occlusion culling is implemented. */
   inline FRenderData
   CullRenderData(const FRenderData& I_Data, const FRenderView& /*I_View*/)
   { return I_Data; }

   /** Convert CPU-side FLight array into GPU-side FGPULight array. */
   inline void
   BuildGPULightArray(const TArray<FLight>& I_Lights, TArray<FGPULight>& O_GPULights)
   {
      O_GPULights.Clear();
      for (const auto& Light : I_Lights)
      {
         O_GPULights.PushBack(FGPULight{
            .Type        = static_cast<UInt32>(Light.Type),
            .Position    = Light.Position,
            .Direction   = Light.Direction,
            .Range       = Light.Range,
            .Color       = FVector4F{Light.Color.R, Light.Color.G, Light.Color.B, Light.Color.A},
            .Intensity   = Light.Intensity,
            .SpotAngle   = Light.SpotAngle,
            .bCastShadow = Light.bCastShadow ? 1u : 0u,
         });
      }
   }

   /** Ensure the light SSBO is large enough, then upload GPU light data.
    *  Reallocates the buffer + descriptor set if the light count exceeds current capacity. */
   inline void
   UploadLightBuffer(FFrameSlotLightBuffer& IO_Slot, FRHI* I_RHI, const TArray<FGPULight>& I_GPULights)
   {
      const UInt64 RequiredCount = I_GPULights.GetSize();
      if (RequiredCount > IO_Slot.CurrentCapacity)
      {
         UInt64 NewCapacity = IO_Slot.CurrentCapacity;
         while (NewCapacity < RequiredCount) { NewCapacity = NewCapacity > 0 ? NewCapacity * 2 : 8; }
         IO_Slot.Buffer = I_RHI->CreateBuffer(FRHIBufferCreateInfo{
            .Size          = NewCapacity * sizeof(FGPULight),
            .Usages        = ERHIBufferUsage::StorageBuffer,
            .bHostWritable = True,
         });
         IO_Slot.DescriptorSet = CreateLightDescriptorSet(I_RHI, IO_Slot.Buffer);
         IO_Slot.CurrentCapacity = NewCapacity;
      }
      if (!I_GPULights.IsEmpty())
      {
         I_RHI->WriteBufferDirect(IO_Slot.Buffer,
            reinterpret_cast<const FByte*>(I_GPULights.Data()),
            I_GPULights.GetSize() * sizeof(FGPULight));
      }
   }

   // =================================================================
   // FGraphicsThread
   // =================================================================

   FGraphics::FGraphicsThread::
   FGraphicsThread(
      FRHI*                                     I_RHI,
      TSPSCChannel<FRenderTask>&                I_ChannelFromMain,
      TAtomic<UInt32>&                          I_PendingDrawRenderTaskCount,
      FEvent*                                   I_FrameConsumedEvent,
      const TArray<FRenderPass>*                I_RenderPasses,
      FRWLock*                                  I_RenderPassesLock,
      const TArray<FNamedTextureDeclaration>*   I_NamedTextureDeclarations,
      FRWLock*                                  I_NamedTexturesLock,
      UInt32                                    I_MaxFrameRate)
       : RHI(I_RHI)
       , ChannelFromMain(I_ChannelFromMain)
       , PendingDrawRenderTaskCount(I_PendingDrawRenderTaskCount)
       , FrameConsumedEvent(I_FrameConsumedEvent)
       , MaxFrameRate(I_MaxFrameRate)
       , RenderPasses(I_RenderPasses)
       , RenderPassesLock(I_RenderPassesLock)
       , NamedTextureDeclarations(I_NamedTextureDeclarations)
       , NamedTexturesLock(I_NamedTexturesLock)
   {
   }

   FGraphics::FSwapChainGraphContext& FGraphics::FGraphicsThread::
   GetOrCreateContext(FRHISwapChainID I_ID)
   {
      VISERA_ASSERT(I_ID < kMaxSwapChainCount && "SwapChain context ID must be < kMaxSwapChainCount");
      while (SwapChainContexts.GetSize() <= I_ID)
      { SwapChainContexts.EmplaceBack(); }
      auto& Context = SwapChainContexts[I_ID];
      if (Context.FrameSlotFrameData.IsEmpty())
      { Context.Init(RHI->GetMaxInFlightFrames(), RHI); }
      return Context;
   }

   void FGraphics::FGraphicsThread::
   Start()
   {
      Thread = MakeUnique<FThread>();
      Thread->Start([this]() { Run(); });
   }

   void FGraphics::FGraphicsThread::
   RequestStop()
   {
      ChannelFromMain.Send(FRenderTask{}); // kInvalidSwapChainID = poison pill
   }

   void FGraphics::FGraphicsThread::
   Join()
   {
      if (Thread) { Thread->Join(); Thread.Reset(); }
   }

   void FGraphics::FGraphicsThread::
   Run()
   {
      LOG_DEBUG("Graphics thread started.");

      while (True)
      {
         // ── Receive & Validate ──────────────────────────────────────
         auto Received = ChannelFromMain.Receive();
         const FRenderTask& RenderTask = Received.GetValue();
         if (RenderTask.SwapChainID == kInvalidSwapChainID)
         {
            if (FrameConsumedEvent) { FrameConsumedEvent->Trigger(); }
            LOG_DEBUG("Graphics thread stopped.");
            break;
         }
         PendingDrawRenderTaskCount.FetchSub(1, EMemoryOrder::Relaxed);
         if (FrameConsumedEvent) { FrameConsumedEvent->Trigger(); }

         FRHISwapChainID SwapChainID = RenderTask.SwapChainID;
         if (SwapChainID == kInvalidSwapChainID) { continue; }
         if (!RHI->IsValidSwapChain(SwapChainID)) { continue; }

         const Bool bHasWindow = RHI->HasWindow(SwapChainID);
         WaitSwapChainReadyIfDirty(SwapChainID);

         // ── BeginFrame ──────────────────────────────────────────────
         FRHITextureID BackBuffer = RHI->BeginFrame(SwapChainID);
         if (BackBuffer.IsNull())
         { LOG_DEBUG("Graphics thread: skipping frame SwapChainID={}, BackBuffer is null.", SwapChainID); continue; }

         auto& SCContext = GetOrCreateContext(SwapChainID);
         const UInt32 MaxInFlight = RHI->GetMaxInFlightFrames();
         const UInt32 FrameSlot = MaxInFlight > 0
            ? static_cast<UInt32>(SCContext.FrameSerial % MaxInFlight)
            : 0;
         ++SCContext.FrameSerial;
         if (bHasWindow && RHI->IsSwapChainDirty(SwapChainID))
         { SCContext.InvalidateCache(); }

         // ═══════ RENDER PIPELINE ════════════════════════════════════

         // Stage 1: Collect (Main thread -> FRenderTask.Data, already done)

         // Stage 2: Cull
         FRenderData CulledData = CullRenderData(RenderTask.Data, RenderTask.RenderView);

         // Stage 3: Batch & Sort
         FRenderList FrameRenderList;
         BatchAndSort(CulledData, FrameRenderList);
         UploadInstanceBuffers(FrameRenderList, RHI, SCContext.FrameSlotInstanceStates[FrameSlot]);

         // Stage 4: Build RenderGraph
         // 4a. Populate engine FrameData UBO (Set 0)
         auto& FrameData = SCContext.FrameSlotFrameData[FrameSlot];
         {
            const auto& View = RenderTask.RenderView;
            const Float RenderWidth  = static_cast<Float>(RenderTask.RenderArea.Width);
            const Float RenderHeight = static_cast<Float>(RenderTask.RenderArea.Height);
            FEngineFrameData EngineData
            {
               .ViewMatrix           = View.ViewMatrix,
               .ProjectionMatrix     = View.ProjectionMatrix,
               .ViewProjectionMatrix = View.ProjectionMatrix * View.ViewMatrix,
               .RenderSize           = FVector4F{ RenderWidth, RenderHeight,
                                                  RenderWidth > 0.f ? 1.f / RenderWidth : 0.f,
                                                  RenderHeight > 0.f ? 1.f / RenderHeight : 0.f },
               .UserFrameData        = RenderTask.UserFrameData,
            };
            RHI->WriteBufferDirect(FrameData.Buffer,
               reinterpret_cast<const FByte*>(&EngineData), sizeof(FEngineFrameData));
         }

         // 4b. Upload light SSBO (Set 2)
         auto& LightSlot = SCContext.FrameSlotLightBuffers[FrameSlot];
         {
            TArray<FGPULight> GPULights;
            BuildGPULightArray(CulledData.Lights, GPULights);
            UploadLightBuffer(LightSlot, RHI, GPULights);
         }

         // 4c. Build RenderGraph, inject textures & passes
         auto Graph = MakeUnique<FRenderGraph>(SwapChainID, BackBuffer);
         auto& CacheSlot = SCContext.InFlightCaches[FrameSlot];
         InjectNamedTextures(Graph.Get(), CacheSlot, RenderTask.RenderArea.Width, RenderTask.RenderArea.Height);

         // Read lock held through graph build + compile + execute to keep raw IRenderPass* valid.
         {
            FScopeReadLock PassesLock(RenderPassesLock);
            if (RenderPasses && !RenderPasses->IsEmpty())
            {
               for (const auto& RegisteredPass : *RenderPasses)
               {
                  LOG_TRACE("Graphics thread: adding pass '{}'.", RegisteredPass.Name.GetNameString());
                  IRenderPass* RawPass = RegisteredPass.Pass.Get();
                  Graph->AddPass(RegisteredPass.Name,
                     [RawPass](FRDGPassBuilder& PB) { RawPass->Setup(PB); },
                     [RawPass](FRDGPassContext& Ctx) { RawPass->Execute(Ctx); });
               }
            }

            // 4d. PresentTransition (barrier-only pass)
            {
               const FGraphicsID BackBufferID = Graph->FindTexture(kBackBufferTextureName);
               if (bHasWindow && BackBufferID.IsValid())
               {
                  Graph->AddPass(EName::PresentTransition,
                     [BackBufferID](FRDGPassBuilder& PB) {
                        PB.Read(BackBufferID, ERGResourceUsage::Present);
                        PB.Write(BackBufferID, ERGResourceUsage::Present);
                     },
                     [](FRDGPassContext&) { });
               }
            }

            // Stage 5: Compile & Submit
            FRenderContext RenderContext
            {
               .RenderList            = &FrameRenderList,
               .RenderView            = &RenderTask.RenderView,
               .RHI                   = RHI,
               .SwapChainID           = SwapChainID,
               .BackBuffer            = BackBuffer,
               .PipelineCache         = &SCContext.PipelineCache,
               .FrameDataUBO          = FrameData.Buffer,
               .FrameDataDescriptorSet = FrameData.DescriptorSet,
               .LightSSBO             = LightSlot.Buffer,
               .LightDescriptorSet    = LightSlot.DescriptorSet,
               .RenderWidth           = RenderTask.RenderArea.Width,
               .RenderHeight          = RenderTask.RenderArea.Height,
            };
            CompileAndSubmit(RenderContext, Graph.Get());
         } // PassesLock released — raw pointers no longer used.

         // ═══════════════════════════════════════════════════════════

         // ── EndFrame (frame pacing is on main thread) ────────────────
         ExportGraphToCache(*Graph, CacheSlot);
         RHI->Present(SwapChainID);
         RHI->EndFrame();
      }
   }

   void FGraphics::FGraphicsThread::
   WaitSwapChainReadyIfDirty(FRHISwapChainID I_SwapChainID)
   {
      if (!RHI->HasWindow(I_SwapChainID) || !RHI->IsSwapChainDirty(I_SwapChainID))
      { return; }
      RHI->WaitDeviceIdle();
      for (UInt32 Waited = 0; Waited < kMaxDirtyWaitMs && RHI->IsSwapChainDirty(I_SwapChainID); Waited += 1)
      { LOG_TRACE("Graphics thread: waiting for swapchain to be ready... ({}/{})", Waited, kMaxDirtyWaitMs); FThread::Sleep(1); }
   }

   void FGraphics::FGraphicsThread::
   CompileAndSubmit(const FRenderContext& RenderContext, FRenderGraph* Graph)
   {
      if (!RenderContext.RHI->IsValidSwapChain(RenderContext.SwapChainID))
      {
         LOG_DEBUG("Graphics thread: skipping Submit/Present for SwapChainID={} (destroyed during frame).", RenderContext.SwapChainID);
         RenderContext.RHI->EndFrame();
         return;
      }
      Graph->Compile(RenderContext.RHI)->Execute(&RenderContext);
   }

   /** Inject declared named textures into the render graph.
    *  1. Reuse cached RHI textures from I_CacheSlot (triple-buffered, same slot as N frames ago).
    *  2. For declared textures not yet cached, create transient entries (allocated during Compile). */
   void FGraphics::FGraphicsThread::
   InjectNamedTextures(FRenderGraph* I_Graph, TMap<FName, FCachedTexture>& I_CacheSlot,
                       UInt32 I_RenderWidth, UInt32 I_RenderHeight)
   {
      if (!I_Graph) { return; }

      // Phase 1: import cached RHI textures whose size still matches
      for (auto& [CachedName, CachedTex] : I_CacheSlot)
      {
         if (CachedTex.Width != I_RenderWidth || CachedTex.Height != I_RenderHeight)
         { continue; }
         auto Imported = I_Graph->RegisterExternalTexture(
            CachedName,
            CachedTex.ID,
            FRDGTextureCreateInfo{
               .Width  = CachedTex.Width,
               .Height = CachedTex.Height,
               .Format = CachedTex.Format,
            },
            CachedTex.KnownLayout);
         LOG_TRACE("Cache import: '{}' RHI={} Layout={} -> GfxID valid={}",
            CachedName.GetNameString(), CachedTex.ID.GetHandle().GetIndex(),
            static_cast<Int32>(CachedTex.KnownLayout), Imported.IsValid());
      }

      // Phase 2: create transient entries for declared named textures not yet in the graph
      TArray<FNamedTextureDeclaration> Declarations;
      {
         FScopeReadLock Lock(NamedTexturesLock);
         if (NamedTextureDeclarations) { Declarations = *NamedTextureDeclarations; }
      }
      for (const auto& Declaration : Declarations)
      {
         if (I_Graph->FindTexture(Declaration.Name).IsValid()) { continue; }
         auto Info = Declaration.Descriptor;
         if (Declaration.bMatchRenderArea)
         {
            Info.Width  = I_RenderWidth;
            Info.Height = I_RenderHeight;
         }
         if (Info.Width == 0 || Info.Height == 0) { continue; }
         (void)I_Graph->CreateTexture(Declaration.Name, Info);
         LOG_TRACE("Named texture inject: '{}' created as transient ({}x{}).",
            Declaration.Name.GetNameString(), Info.Width, Info.Height);
      }
   }

   /** Persist named textures from the compiled render graph into the cache slot.
    *  When the same triple-buffer slot is reused in a future frame,
    *  InjectNamedTextures will register them as external to avoid reallocation. */
   void FGraphics::FGraphicsThread::
   ExportGraphToCache(FRenderGraph& I_Graph, TMap<FName, FCachedTexture>& I_CacheSlot)
   {
      I_CacheSlot.Clear();
      for (const auto& [Name, GfxID] : I_Graph.GetNamedTextures())
      {
         const auto* Entry = I_Graph.GetTextureEntry(GfxID);
         if (!Entry || !I_Graph.IsTextureLive(GfxID) || Entry->GetRHIID().IsNull()) { continue; }
         LOG_TRACE("Cache export: '{}' -> RHI={} Layout={} (external={})",
            Name.GetNameString(), Entry->GetRHIID().GetHandle().GetIndex(),
            static_cast<int>(Entry->GetKnownLayout()),
            Entry->IsExternal());
         const auto& Info = Entry->GetCreateInfo();
         I_CacheSlot.InsertOrAssign(Name,
            FCachedTexture{
               Entry->GetRHIID(),
               Info.Width,
               Info.Height,
               Info.Format,
               ERHIImageLayout::Undefined,
            });
      }
   }

   void FGraphics::FGraphicsThread::
   ApplyFramePacing(FHighResTimePoint I_FrameStart)
   {
      if (MaxFrameRate == 0) { return; }
      UInt32 const TargetMs = 1000 / MaxFrameRate;
      auto Elapsed = FramePacingClock.Now() - I_FrameStart;
      UInt32 ElapsedMs = Elapsed.Milliseconds();
      if (ElapsedMs + 2U < TargetMs)
      { FThread::Sleep(TargetMs - ElapsedMs - 2U); }
      while ((FramePacingClock.Now() - I_FrameStart).Milliseconds() < TargetMs)
      { /* spin last ~2ms for precise pacing */ }
   }

   // =================================================================
   // FGraphics — public API
   // =================================================================

   void FGraphics::
   UnregisterWindow(FWindow* I_Window)
   {
      if (!I_Window || !RHI) { return; }
      RHI->DestroySwapChain(I_Window);
      for (size_t i = 0; i < ManagedWindows.GetSize(); ++i)
      {
         if (ManagedWindows[i] == I_Window) { ManagedWindows.RemoveAtSwap(static_cast<UInt32>(i)); break; }
      }
   }

   FRHISwapChainID FGraphics::
   RegisterHeadless()
   {
      FRHISwapChainID ID = RHI->CreateSwapChain(nullptr);
      if (ID != kInvalidSwapChainID)
      {
         VISERA_ASSERT(!ManagedHeadlessIDs.IsFull());
         ManagedHeadlessIDs.PushBack(ID);
         LOG_INFO("Registered headless rendering context (id:{}).", ID);
      }
      else
      {
         LOG_ERROR("Failed to register headless rendering context.");
      }
      return ID;
   }

   void FGraphics::
   SetCamera(FCamera I_Camera)
   {
      FrameCamera = std::move(I_Camera);
   }

   void FGraphics::
   SubmitLight(FLight I_Light)
   {
      FrameLights.PushBack(std::move(I_Light));
   }

   void FGraphics::
   Draw(const IRenderable& I_Renderable)
   {
      PendingDraws.PushBack(I_Renderable.ToRenderableMeta());
   }

   void FGraphics::
   Draw(const FRenderableMeta& I_Meta)
   {
      PendingDraws.PushBack(I_Meta);
   }

   void FGraphics::
   Render(FWindow* I_Window)
   {
      if (!I_Window || !RHI)
      { return; }
      FRHISwapChainID SwapChainID = RHI->QuerySwapChainID(I_Window);
      if (SwapChainID == kInvalidSwapChainID)
      {
         LOG_TRACE("Triggered the eager creation of the swapchain.");
         SwapChainID = RHI->CreateSwapChain(I_Window);
         if (SwapChainID == kInvalidSwapChainID) { return; }
         VISERA_ASSERT(!ManagedWindows.IsFull());
         ManagedWindows.PushBack(I_Window);
      }
      RHI->UpdateSwapChainMinimized(SwapChainID, I_Window->IsMinimized());

      FRenderArea RenderArea;
      RenderArea.Width  = I_Window->GetWidth()  > 0 ? static_cast<UInt32>(I_Window->GetWidth())  : 1u;
      RenderArea.Height = I_Window->GetHeight() > 0 ? static_cast<UInt32>(I_Window->GetHeight()) : 1u;

      while (PendingDrawRenderTaskCount.Load(EMemoryOrder::Relaxed) >= kMaxPendingDrawRenderTasks)
      {
         if (bShuttingDown.Load(EMemoryOrder::Relaxed)) { return; }
         FrameConsumedEvent.WaitAndReset();
      }
      PendingDrawRenderTaskCount.FetchAdd(1, EMemoryOrder::Relaxed);

      FRenderView RenderView;
      if (FrameCamera.HasValue())
      {
         const FCamera& Camera = FrameCamera.GetValue();
         RenderView.ViewMatrix       = Camera.GetViewMatrix();
         RenderView.ProjectionMatrix = Camera.GetProjectionMatrix();
      }

      FRenderData RenderData;
      RenderData.DrawCommands = std::move(PendingDraws);
      RenderData.Lights       = std::move(FrameLights);
      FrameCamera.Reset();
      FrameLights.Clear();
      PendingDraws.Clear();

      FRenderTask Task
      {
         .SwapChainID   = SwapChainID,
         .Data          = std::move(RenderData),
         .RenderView    = std::move(RenderView),
         .RenderArea    = RenderArea,
         .UserFrameData = UserFrameData.Load(EMemoryOrder::Relaxed),
      };
      ChannelToGraphics.Send(std::move(Task));
   }

   Float FGraphics::
   GetFrameRate(FWindow* I_Window) const
   {
      return RHI ? RHI->GetFrameRate(I_Window) : 0.f;
   }

   TSharedPtr<FMaterial> FGraphics::
   LoadMaterial(const VPath& I_MaterialPath)
   {
      const FName Key = I_MaterialPath.GetName();
      auto It = MaterialCache.Find(Key);
      if (It != MaterialCache.end())
         return It->second;

      const FPath ResolvedPath = I_MaterialPath.GetRealPath();
      auto JSONOpt = FJSON::Load(ResolvedPath);
      if (!JSONOpt.HasValue())
      {
         LOG_ERROR(
            "LoadMaterial: could not load material {} (resolved: {}). For @assets:// paths, keep the "
            "'assets' directory next to the executable (ship the full build output or installer layout; "
            "do not run from inside an archive without extracting).",
            I_MaterialPath,
            ResolvedPath);
         return nullptr;
      }
      auto Material = FMaterial::Create(JSONOpt.GetValue(), AssetHub, RHI);
      if (!Material)
         return nullptr;
      LOG_INFO("LoadMaterial: {} loaded successfully.", I_MaterialPath);
      MaterialCache[Key] = Material;
      return Material;
   }

}
