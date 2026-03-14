module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics;
#define VISERA_MODULE_NAME "Runtime.Graphics"
export import Visera.Runtime.Graphics.Scene;
export import Visera.Runtime.Graphics.Framework;
export import Visera.Runtime.Graphics.Material;
export import Visera.Runtime.Graphics.PipelineCache;
export import Visera.Runtime.Graphics.RenderGraph;
       import Visera.Runtime.Global;
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
   /** A registered pass factory entry. User, UI, debug overlays all use this same type. */
   struct VISERA_RUNTIME_API FRenderPass
   {
      UInt32  Priority {0};
      FName   Name;
      TFunction<void(FRenderGraph&, const FRenderContext&)> Execute;
   };

   /** Hash (Material*, Mesh*) pair into a single UInt64 for batch-key dedup. */
   [[nodiscard]] inline UInt64
   MakeBatchKey(const FMaterial* I_Mat, const FMesh* I_Mesh) noexcept
   {
      return Math::GoldenRatioHashCombine(
         reinterpret_cast<UInt64>(I_Mat),
         reinterpret_cast<UInt64>(I_Mesh));
   }

   /** Fills O_List by iterating I_Data, batching by (Material, PSO, Mesh). Routes by surface (Opaque/Transparent). */
   inline void
   ExtractAndSortDrawList(const FRenderData&        I_Data,
                          FRenderList&              O_List,
                          FPipelineCache*           I_PipelineCache,
                          FRHI*                     I_RHI,
                          const TArray<ERHIFormat>&  I_ColorFormats,
                          ERHIFormat                I_DepthFormat)
   {
      O_List.OpaqueBatches.Clear();
      O_List.TransparentBatches.Clear();
      O_List.WireframeBatches.Clear();
      // Batch key hash(Material + Mesh) -> batch index, one map per surface type.
      TMap<UInt64, UInt32> OpaqueMap;
      TMap<UInt64, UInt32> TransparentMap;
      const auto& Renderables = I_Data.GetRenderables();
      for (const auto& R : Renderables)
      {
         const FMaterial* Mat = R->GetMaterial().Get();
         if (!Mat || !Mat->IsValid()) { continue; }
         const FInstanceData InstanceData = R->GetInstanceData();
         auto MeshPtr = R->GetMesh();
         const UInt64 Key = MakeBatchKey(Mat, MeshPtr.Get());
         const ESurfaceType Surface = Mat->GetSurface();
         const Bool bTransparent = (Surface == ESurfaceType::Transparent);
         TArray<FRenderBatch>& FillBatches = bTransparent ? O_List.TransparentBatches : O_List.OpaqueBatches;
         TMap<UInt64, UInt32>& FillMap = bTransparent ? TransparentMap : OpaqueMap;
         auto FillIt = FillMap.Find(Key);
         if (FillIt == FillMap.end())
         {
            FRHIRenderPassID Pipeline = I_PipelineCache->GetOrCreate(I_RHI, Mat, I_ColorFormats, I_DepthFormat);
            FRenderBatch Batch {
               .Pipeline = Pipeline,
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

   /** Creates per-batch SSBO + descriptor set for FInstanceData[].
    *  Called once per frame after ExtractAndSortDrawList, before RDG execution.
    *  Buffer lifetime is tied to FRHIBufferID RAII -- destroyed when FRenderList goes out of scope. */
   inline void
   UploadInstanceBuffers(FRenderList& IO_List, FRHI* I_RHI)
   {
      auto UploadBatches = [I_RHI](TArray<FRenderBatch>& Batches)
      {
         for (auto& Batch : Batches)
         {
            if (Batch.Instances.IsEmpty()) { continue; }
            const UInt64 Size = Batch.Instances.GetSize() * sizeof(FInstanceData);
            Batch.InstanceBuffer = I_RHI->CreateBuffer(FRHIBufferCreateInfo{
               .Size   = Size,
               .Usages = ERHIBufferUsage::StorageBuffer | ERHIBufferUsage::TransferDst,
            });
            I_RHI->UploadBuffer(Batch.InstanceBuffer,
               reinterpret_cast<const FByte*>(Batch.Instances.Data()), Size, 0);

            Batch.InstanceDescriptorSet = I_RHI->CreateDescriptorSet(FRHIDescriptorSetCreateInfo{
               .Bindings = {{
                  .Binding = 0,
                  .Type    = ERHIDescriptorType::StorageBuffer,
                  .Count   = 1,
                  .Stages  = ERHIShaderStage::Vertex | ERHIShaderStage::Fragment,
               }},
            });
            I_RHI->WriteDescriptorStorageBuffer(
               Batch.InstanceDescriptorSet, 0, Batch.InstanceBuffer);
         }
      };
      UploadBatches(IO_List.OpaqueBatches);
      UploadBatches(IO_List.TransparentBatches);
   }

   class VISERA_RUNTIME_API FGraphics : public IRuntimeService
   {
   public:
      /** Convenience: resolve swap chain from I_Window; I_RenderArea default {} -> derive from window size. Forwards to Render(I_SwapChainID, I_Scene, I_RenderArea). Lazily creates swap chain on first call. */
      void
      Render(FWindow* I_Window, const FScene& I_Scene,
             const FRenderArea& I_RenderArea = {});

      /** Unified render entry (headless or after window overload). I_RenderArea.Width/Height 0 -> SwapChain size. */
      void
      Render(FRHISwapChainID I_SwapChainID, const FScene& I_Scene,
             const FRenderArea& I_RenderArea);

      /** Create a headless rendering context. Returns a stable SwapChainID. */
      [[nodiscard]] FRHISwapChainID
      RegisterHeadless();

      /** Unregister a window and destroy its swap chain. Call before destroying the FWindow (e.g. before app terminate) to avoid dangling pointers. */
      void
      UnregisterWindow(FWindow* I_Window);

      /** Register a pass factory. Lower priority values execute first. All pass sources (user, UI, debug) use this same API. */
      void
      RegisterPass(UInt32 I_Priority, FName I_Name,
                   TFunction<void(FRenderGraph&, const FRenderContext&)> I_Execute);

      [[nodiscard]] TSharedPtr<FMaterial>
      LoadMaterial(const FPath& I_MaterialFile);

      [[nodiscard]] const FRHI*
      GetRHI() const { return RHI.Get(); }

      /** Frame rate (FPS) for I_Window from RHI (swap chain present timing). 0 if I_Window is null or no windowed swap chain. */
      [[nodiscard]] Float
      GetFrameRate(FWindow* I_Window) const;

      ~FGraphics();

   private:

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
         FPipelineCache                        PipelineCache;

         void Init(UInt32 I_MaxInFlightFrames)
         {
            InFlightCaches.Resize(I_MaxInFlightFrames);
         }

         void InvalidateCache()
         {
            for (auto& Slot : InFlightCaches) { Slot.Clear(); }
         }
      };

      /** Dedicated thread: take draw intent from channel -> BeginFrame -> call pass factories -> PresentTransition -> Compile -> Execute -> Present -> EndFrame. */
      struct VISERA_RUNTIME_API FGraphicsThread
      {
      public:
         FGraphicsThread(
            FRHI*                         I_RHI,
            TSPSCChannel<FRenderTask>&    I_ChannelFromMain,
            TAtomic<UInt32>&              I_PendingDrawRenderTaskCount,
            FEvent*                       I_FrameConsumedEvent,
            const TArray<FRenderPass>*    I_RenderPasses,
            FRWLock*                      I_RenderPassesLock,
            UInt32                        I_MaxFrameRate);

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
         void
         ImportCachedTextures(FRenderGraph* I_Graph, TMap<FName, FCachedTexture>& I_CacheSlot,
                              UInt32 I_RenderWidth, UInt32 I_RenderHeight);
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
         UInt32                                MaxFrameRate {0};
         FHiResClock                           FramePacingClock;
         TUniquePtr<FThread>                   Thread;
         const TArray<FRenderPass>*            RenderPasses {nullptr};
         FRWLock*                              RenderPassesLock {nullptr};

         TInlineArray<FSwapChainGraphContext, kMaxSwapChainCount> SwapChainContexts;
      };

      TSharedPtr<FAssetHub>                     AssetHub;
      TSharedPtr<FRHI>                          RHI;
      TSPSCChannel<FRenderTask>                 ChannelToGraphics;
      TAtomic<UInt32>                           PendingDrawRenderTaskCount {0};
      FEvent                                    FrameConsumedEvent;
      TAtomic<Bool>                             bShuttingDown {False};

      TUniquePtr<FGraphicsThread>               GraphicsThread;
      mutable FRWLock                           RenderPassesLock;
      TArray<FRenderPass>                       RenderPasses;

      TInlineArray<FRHISwapChainID, kMaxSwapChainCount> ManagedHeadlessIDs;
      TInlineArray<FWindow*, kMaxSwapChainCount>        ManagedWindows;

   public:
      FGraphics(FString I_Name, FServiceRegistry* I_Registry, FJSONView I_ConfigView,
                TMulticastDelegate<const FJSONRoute&>* I_OnConfigChange, FStringView I_RuntimeName)
          : IRuntimeService(I_Name, I_Registry, std::move(I_ConfigView), I_OnConfigChange, I_RuntimeName)
      {
         Dependencies =
         {
            EService::AssetHub,
            EService::RHI,
         };

         if (!OnBootstrap.TryBind([this]
         {
            if (auto RHIWeak = GetService<FRHI>(EService::RHI); auto RHIShared = RHIWeak.Lock())
            { RHI = RHIShared; }
            else
            { LOG_FATAL("Failed to get RHI service!"); return False; }

            if (auto AHWeak = GetService<FAssetHub>(EService::AssetHub); auto AHShared = AHWeak.Lock())
            { AssetHub = AHShared; }
            else
            { LOG_FATAL("Failed to get AssetHub service!"); return False; }

            if (!GraphicsThread)
            {
               UInt32 MaxFrameRate = GetConfig().GetNumber(TJSONRoute<"Graphics.MaxFrameRate">(), 0);
               GraphicsThread = MakeUnique<FGraphicsThread>(
                  RHI.Get(), ChannelToGraphics, PendingDrawRenderTaskCount,
                  &FrameConsumedEvent, &RenderPasses, &RenderPassesLock, MaxFrameRate);
               GraphicsThread->Start();
               if (MaxFrameRate > 0) { LOG_INFO("Graphics: thread started (MaxFrameRate={}).", MaxFrameRate); }
               else { LOG_INFO("Graphics: thread started (MaxFrameRate=Unlimited)."); }
            }
            return True;
         }))
         { LOG_FATAL("Failed to bind bootstrap function!"); }

         if (!OnTerminate.TryBind([this]
         {
            // Stop graphics thread first so it does not enter the long kMaxDirtyWaitMs
            // loop after we mark swap chains destroyed (which makes IsSwapChainDirty True).
            if (GraphicsThread)
            {
               bShuttingDown.Store(True, EMemoryOrder::Relaxed);
               FrameConsumedEvent.Trigger();
               GraphicsThread->RequestStop();
               GraphicsThread->Join();
               GraphicsThread.Reset();
            }
            if (RHI) { RHI->WaitDeviceIdle(); }
            for (FWindow* W : ManagedWindows)
            { RHI->DestroySwapChain(W); }
            for (auto Id : ManagedHeadlessIDs)
            { RHI->DestroySwapChain(Id); }
            ManagedWindows.Clear();
            ManagedHeadlessIDs.Clear();
            LOG_DEBUG("Graphics: terminated.");
            return True;
         }))
         { LOG_FATAL("Failed to bind terminate function!"); }
      }
   };

   // =================================================================
   // RegisterPass
   // =================================================================

   void FGraphics::
   RegisterPass(UInt32 I_Priority, FName I_Name,
                TFunction<void(FRenderGraph&, const FRenderContext&)> I_Execute)
   {
      // Write-lock: RegisterPass may be called from the main thread while
      // the graphics thread is reading the array (step 3 in Run()).
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
         .Execute  = std::move(I_Execute),
      });
      LOG_INFO("Graphics::RegisterPass: '{}' registered at priority {}.", I_Name.GetNameString(), I_Priority);
   }

   // =================================================================
   // FGraphicsThread
   // =================================================================

   FGraphics::FGraphicsThread::
   FGraphicsThread(
      FRHI*                                 I_RHI,
      TSPSCChannel<FRenderTask>& I_ChannelFromMain,
      TAtomic<UInt32>&                      I_PendingDrawRenderTaskCount,
      FEvent*                               I_FrameConsumedEvent,
      const TArray<FRenderPass>*            I_RenderPasses,
      FRWLock*                              I_RenderPassesLock,
      UInt32                                I_MaxFrameRate)
       : RHI(I_RHI)
       , ChannelFromMain(I_ChannelFromMain)
       , PendingDrawRenderTaskCount(I_PendingDrawRenderTaskCount)
       , FrameConsumedEvent(I_FrameConsumedEvent)
       , MaxFrameRate(I_MaxFrameRate)
       , RenderPasses(I_RenderPasses)
       , RenderPassesLock(I_RenderPassesLock)
   {
      for (UInt32 i = 0; i < kMaxSwapChainCount; ++i)
      {
         SwapChainContexts.EmplaceBack();
         SwapChainContexts.Back().Init(kMaxInFlightFrames);
      }
   }

   FGraphics::FSwapChainGraphContext& FGraphics::FGraphicsThread::
   GetOrCreateContext(FRHISwapChainID I_ID)
   {
      VISERA_ASSERT(I_ID < SwapChainContexts.GetSize() && "SwapChain context ID must be < kMaxSwapChainCount");
      return SwapChainContexts[I_ID];
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
      LOG_DEBUG("({}) Graphics thread started.", RHI->GetRuntimeName());

      while (True)
      {
         // 1. Receive task (SwapChainID == kInvalidSwapChainID = poison pill, stop)
         auto Received = ChannelFromMain.Receive();
         const FRenderTask& RenderTask = Received.GetValue();
         if (RenderTask.SwapChainID == kInvalidSwapChainID)
         {
            if (FrameConsumedEvent) { FrameConsumedEvent->Trigger(); }
            LOG_DEBUG("({}) Graphics thread stopped.", RHI->GetRuntimeName());
            break;
         }
         PendingDrawRenderTaskCount.FetchSub(1, EMemoryOrder::Relaxed);
         if (FrameConsumedEvent) { FrameConsumedEvent->Trigger(); }
         auto FrameStart = FramePacingClock.Now();

         // 2. Validate swap chain
         FRHISwapChainID SwapChainID = RenderTask.SwapChainID;
         if (SwapChainID == kInvalidSwapChainID) { continue; }
         if (!RHI->IsValidSwapChain(SwapChainID)) { continue; }

         // 3. Snapshot passes under read lock.
         // Copy into a local array so the graphics thread iterates a stable set
         // even if the main thread calls RegisterPass() concurrently.
         TArray<FRenderPass> FramePasses;
         {
            FScopeReadLock Lock(RenderPassesLock);
            if (!RenderPasses || RenderPasses->IsEmpty())
            {
               LOG_TRACE("Graphics thread: no passes registered, skipping frame.");
               continue;
            }
            FramePasses = *RenderPasses;
         }

         // 4. Wait swap chain if dirty
         const Bool bHasWindow = RHI->HasWindow(SwapChainID);
         WaitSwapChainReadyIfDirty(SwapChainID);

         // 5. BeginFrame
         FRHITextureID BackBuffer = RHI->BeginFrame(SwapChainID);
         if (BackBuffer.IsNull())
         { LOG_DEBUG("Graphics thread: skipping frame SwapChainID={}, BackBuffer is null (swapchain destroyed or unavailable).", SwapChainID); continue; }

         // 6. Extract draw list and build context
         auto& SCContext = GetOrCreateContext(SwapChainID);
         FRenderList FrameRenderList;
         static constexpr ERHIFormat kSceneColorFormat = ERHIFormat::R8G8B8A8_UNorm;
         ExtractAndSortDrawList(RenderTask.Data, FrameRenderList, &SCContext.PipelineCache, RHI,
                               TArray<ERHIFormat>{kSceneColorFormat}, ERHIFormat::Undefined);
         UploadInstanceBuffers(FrameRenderList, RHI);
         FRenderContext RenderContext
         {
            .RenderList     = &FrameRenderList,
            .RenderView     = &RenderTask.RenderView,
            .RHI            = RHI,
            .SwapChainID    = SwapChainID,
            .BackBuffer     = BackBuffer,
            .PipelineCache  = &SCContext.PipelineCache,
            .RenderWidth    = RenderTask.RenderArea.Width,
            .RenderHeight   = RenderTask.RenderArea.Height,
         };
         auto Graph = MakeUnique<FRenderGraph>(SwapChainID, BackBuffer);
         const UInt32 MaxInFlight = RHI->GetMaxInFlightFrames();
         const UInt32 FrameSlot = MaxInFlight > 0
            ? static_cast<UInt32>(SCContext.FrameSerial % MaxInFlight)
            : 0;
         ++SCContext.FrameSerial;
         if (bHasWindow && RHI->IsSwapChainDirty(SwapChainID))
         { SCContext.InvalidateCache(); }
         auto& CacheSlot = SCContext.InFlightCaches[FrameSlot];

         // 7. Import cached textures
         ImportCachedTextures(Graph.Get(), CacheSlot, RenderTask.RenderArea.Width, RenderTask.RenderArea.Height);

         // 8. Run passes
         for (const auto& RenderPass : FramePasses)
         {
            LOG_TRACE("Graphics thread: running pass factory '{}'.", RenderPass.Name.GetNameString());
            RenderPass.Execute(*Graph, RenderContext);
         }

         // 9. PresentTransition — barrier-only pass (empty execute lambda).
         // Both Read and Write are declared so TopologicalSort establishes a
         // dependency on whatever pass last wrote the backbuffer (e.g. FinalBlit),
         // and RDG inserts the TransferDst -> PresentSrc layout transition.
         if (bHasWindow && Graph->HasBackBuffer())
         {
            auto BB = Graph->GetBackBuffer();
            Graph->AddPass(EName::PresentTransition,
               [BB](FRDGPassBuilder& PB) {
                  PB.Read(BB, ERGResourceUsage::Present);
                  PB.Write(BB, ERGResourceUsage::Present);
               },
               [](FRDGPassContext&) { });
         }

         // 10. Compile, execute, submit
         CompileAndSubmit(RenderContext, Graph.Get());

         // 11. Export cache and present
         ExportGraphToCache(*Graph, CacheSlot);
         RHI->Present(SwapChainID);
         RHI->EndFrame();

         // 12. Frame pacing
         ApplyFramePacing(FrameStart);
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

   void FGraphics::FGraphicsThread::
   ImportCachedTextures(FRenderGraph* I_Graph, TMap<FName, FCachedTexture>& I_CacheSlot,
                       UInt32 I_RenderWidth, UInt32 I_RenderHeight)
   {
      if (!I_Graph) { return; }
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
   }

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
   Render(FWindow* I_Window, const FScene& I_Scene,
          const FRenderArea& I_RenderArea)
   {
      if (!I_Window)
      {
         LOG_WARN("FGraphics::Render called with nullptr window. Use RegisterHeadless() + Render(SwapChainID) for headless rendering.");
         return;
      }
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

      FRenderArea RenderArea = I_RenderArea;
      if (RenderArea.Width == 0 && RenderArea.Height == 0)
      {
         RenderArea.Width  = I_Window->GetWidth()  > 0 ? static_cast<UInt32>(I_Window->GetWidth())  : 1u;
         RenderArea.Height = I_Window->GetHeight() > 0 ? static_cast<UInt32>(I_Window->GetHeight()) : 1u;
      }
      Render(SwapChainID, I_Scene, RenderArea);
   }

   void FGraphics::
   Render(FRHISwapChainID I_SwapChainID, const FScene& I_Scene,
          const FRenderArea& I_RenderArea)
   {
      if (I_SwapChainID == kInvalidSwapChainID)
      {
         LOG_WARN("FGraphics::Render called with invalid SwapChainID.");
         return;
      }
      while (PendingDrawRenderTaskCount.Load(EMemoryOrder::Relaxed) >= kMaxPendingDrawRenderTasks)
      {
         if (bShuttingDown.Load(EMemoryOrder::Relaxed)) { return; }
         FrameConsumedEvent.WaitAndReset();
      }
      PendingDrawRenderTaskCount.FetchAdd(1, EMemoryOrder::Relaxed);
      FRenderData RenderData;
      FRenderView RenderView;
      I_Scene.BuildRenderData(RenderData);
      RenderView = I_Scene.BuildRenderView();
      FRenderTask Task{I_SwapChainID, std::move(RenderData), std::move(RenderView), I_RenderArea};
      ChannelToGraphics.Send(std::move(Task));
   }

   Float FGraphics::
   GetFrameRate(FWindow* I_Window) const
   {
      return RHI ? RHI->GetFrameRate(I_Window) : 0.f;
   }

   TSharedPtr<FMaterial> FGraphics::
   LoadMaterial(const FPath& I_MaterialFile)
   {
      auto JSONOpt = FJSON::Load(I_MaterialFile);
      if (!JSONOpt.HasValue())
      { LOG_ERROR("LoadMaterial: failed to parse {}.", I_MaterialFile); return nullptr; }
      auto Material = FMaterial::Create(JSONOpt.GetValue(), AssetHub.Get(), RHI.Get(), I_MaterialFile);
      if (Material)
      { LOG_INFO("LoadMaterial: {} loaded successfully.", I_MaterialFile); }
      return Material;
   }

   FGraphics::~FGraphics()
   {
      PROFILING_ONLY_FIELD(LogRenderGraphCompileProfilingSummary();)
   }
}
