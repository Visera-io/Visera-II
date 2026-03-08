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

export namespace Visera
{
   /** A registered pass factory entry. User, UI, debug overlays all use this same type. */
   struct VISERA_RUNTIME_API FPassEntry
   {
      UInt32  Priority {0};
      FName   Name;
      TFunction<void(FRenderGraph&, const FRenderContext&)> Factory;
   };

   class VISERA_RUNTIME_API FGraphics : public IRuntimeService
   {
   public:
      /** Render a windowed frame. Lazily creates swap chain for I_Window on first call. */
      void
      Render(FWindow* I_Window, const FScene& I_Scene);

      /** Render a headless (or direct ID-based) frame. Use the ID returned by RegisterHeadless(). */
      void
      Render(FRHISwapChainID I_SwapChainID, const FScene& I_Scene);

      /** Create a headless rendering context. Returns a stable SwapChainID. */
      [[nodiscard]] FRHISwapChainID
      RegisterHeadless();

      /** Unregister a window and destroy its swap chain. Call before destroying the FWindow (e.g. before app terminate) to avoid dangling pointers. */
      void
      UnregisterWindow(FWindow* I_Window);

      /** Register a pass factory. Lower priority values execute first. All pass sources (user, UI, debug) use this same API. */
      void
      RegisterPass(UInt32 I_Priority, FName I_Name,
                   TFunction<void(FRenderGraph&, const FRenderContext&)> I_Factory);

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
         UInt32          Width  {0};
         UInt32          Height {0};
         ERHIFormat      Format {ERHIFormat::Undefined};
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
            FRHI*                                 I_RHI,
            TSPSCChannel<TOptional<FRenderTask>>& I_ChannelFromMain,
            TAtomic<UInt32>&                      I_PendingDrawRenderTaskCount,
            const TArray<FPassEntry>*             I_PassEntries,
            UInt32                                I_MaxFrameRate);

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

         FRHI*                                 RHI {nullptr};
         TSPSCChannel<TOptional<FRenderTask>>& ChannelFromMain;
         TAtomic<UInt32>&                      PendingDrawRenderTaskCount;
         UInt32                                MaxFrameRate {0};
         FHiResClock                           FramePacingClock;
         TUniquePtr<FThread>                   Thread;
         const TArray<FPassEntry>*             PassEntries {nullptr};

         TArray<FSwapChainGraphContext>         SwapChainContexts;
      };

      TSharedPtr<FAssetHub>                    AssetHub;
      TSharedPtr<FRHI>                         RHI;
      TSPSCChannel<TOptional<FRenderTask>>     ChannelToGraphics;
      TAtomic<UInt32>                          PendingDrawRenderTaskCount {0};
      TArray<FWindow*>                        ManagedWindows;
      TUniquePtr<FGraphicsThread>              GraphicsThread;
      TArray<FRHISwapChainID>                  ManagedHeadlessIDs;
      TArray<FPassEntry>                       PassEntries;

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
                  &PassEntries, MaxFrameRate);
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
            // loop after we mark swap chains destroyed (which makes IsSwapChainDirty true).
            if (GraphicsThread)
            {
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
                TFunction<void(FRenderGraph&, const FRenderContext&)> I_Factory)
   {
      UInt64 InsertIdx = 0;
      for (; InsertIdx < PassEntries.GetSize(); ++InsertIdx)
      {
         if (PassEntries[InsertIdx].Priority > I_Priority) { break; }
      }
      PassEntries.Insert(PassEntries.begin() + InsertIdx, FPassEntry{
         .Priority = I_Priority,
         .Name     = I_Name,
         .Factory  = std::move(I_Factory),
      });
      LOG_INFO("Graphics::RegisterPass: '{}' registered at priority {}.", I_Name.GetNameString(), I_Priority);
   }

   // =================================================================
   // FGraphicsThread
   // =================================================================

   FGraphics::FGraphicsThread::
   FGraphicsThread(
      FRHI*                                 I_RHI,
      TSPSCChannel<TOptional<FRenderTask>>& I_ChannelFromMain,
      TAtomic<UInt32>&                      I_PendingDrawRenderTaskCount,
      const TArray<FPassEntry>*             I_PassEntries,
      UInt32                                I_MaxFrameRate)
       : RHI(I_RHI)
       , ChannelFromMain(I_ChannelFromMain)
       , PendingDrawRenderTaskCount(I_PendingDrawRenderTaskCount)
       , MaxFrameRate(I_MaxFrameRate)
       , PassEntries(I_PassEntries)
   {
      SwapChainContexts.Resize(kInvalidSwapChainID);
      for (auto& Ctx : SwapChainContexts)
      { Ctx.Init(kMaxInFlightFrames); }
   }

   FGraphics::FSwapChainGraphContext& FGraphics::FGraphicsThread::
   GetOrCreateContext(FRHISwapChainID I_ID)
   {
      if (I_ID >= SwapChainContexts.GetSize())
      {
         SwapChainContexts.Resize(static_cast<UInt32>(I_ID) + 1);
         SwapChainContexts[I_ID].Init(kMaxInFlightFrames);
      }
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
      ChannelFromMain.Send(TOptional<FRenderTask>{});
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
         auto Received = ChannelFromMain.Receive();
         if (!Received.GetValue().HasValue())
         {
            LOG_DEBUG("({}) Graphics thread stopped.", RHI->GetRuntimeName());
            break;
         }

         auto FrameStart = FramePacingClock.Now();
         PendingDrawRenderTaskCount.FetchSub(1, EMemoryOrder::Relaxed);
         const FRenderTask& RenderTask = Received.GetValue().GetValue();

         FRHISwapChainID SwapChainID = RenderTask.SwapChainID;
         if (SwapChainID == kInvalidSwapChainID) { continue; }
         if (!RHI->IsValidSwapChain(SwapChainID)) { continue; }

         if (!PassEntries || PassEntries->IsEmpty())
         {
            LOG_TRACE("Graphics thread: no passes registered, skipping frame.");
            continue;
         }

         const Bool bHasWindow = RHI->HasWindow(SwapChainID);

         if (bHasWindow && RHI->IsSwapChainDirty(SwapChainID))
         {
            RHI->WaitDeviceIdle();
            for (UInt32 Waited = 0; Waited < kMaxDirtyWaitMs && RHI->IsSwapChainDirty(SwapChainID); Waited += 1)
            { LOG_TRACE("Graphics thread: waiting for swapchain to be ready... ({}/{})", Waited, kMaxDirtyWaitMs); FThread::Sleep(1); }
         }

         FRHITextureID BackBuffer = RHI->BeginFrame(SwapChainID);
         if (BackBuffer.IsNull())
         { LOG_DEBUG("Graphics thread: skipping frame SwapChainID={}, BackBuffer is null (swapchain destroyed or unavailable).", SwapChainID); continue; }

         auto& SCContext = GetOrCreateContext(SwapChainID);

         FRenderContext Ctx{
            .RHI            = RHI,
            .Data           = &RenderTask.Data,
            .RenderView     = &RenderTask.RenderView,
            .SwapChainID    = SwapChainID,
            .BackBuffer     = BackBuffer,
            .PipelineCache  = &SCContext.PipelineCache,
            .ViewportWidth  = RenderTask.ViewportWidth,
            .ViewportHeight = RenderTask.ViewportHeight,
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
         for (auto& [CachedName, CachedTex] : CacheSlot)
         {
            if (CachedTex.Width  == RenderTask.ViewportWidth &&
                CachedTex.Height == RenderTask.ViewportHeight)
            {
               auto Imported = Graph->RegisterExternalTexture(
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
                  static_cast<int>(CachedTex.KnownLayout), Imported.IsValid());
            }
            else
            {
               LOG_DEBUG("Cache skip '{}': size mismatch (cached={}x{} vs viewport={}x{})",
                  CachedName.GetNameString(), CachedTex.Width, CachedTex.Height,
                  RenderTask.ViewportWidth, RenderTask.ViewportHeight);
            }
         }

         for (const auto& Entry : *PassEntries)
         {
            LOG_TRACE("Graphics thread: running pass factory '{}'.", Entry.Name.GetNameString());
            Entry.Factory(*Graph, Ctx);
         }

         if (bHasWindow && Graph->HasBackBuffer())
         {
            auto BB = Graph->GetBackBuffer();
            Graph->AddPass("PresentTransition",
               [BB](FRDGPassBuilder& PB) {
                  PB.Read(BB, ERGResourceUsage::Present);
                  PB.Write(BB, ERGResourceUsage::Present);
               },
               [](const FRDGPassContext&) { });
         }

         auto CmdList = RHI->CreateCommandList();
         Graph->Compile(RHI)->Execute(&CmdList);
         RHI->Submit(std::move(CmdList));

         CacheSlot.Clear();
         for (const auto& [Name, GfxID] : Graph->GetNamedTextures())
         {
            const auto* Entry = Graph->GetTextureEntry(GfxID);
            if (!Entry || !Graph->IsTextureLive(GfxID) || Entry->GetRHIID().IsNull()) { continue; }
            LOG_TRACE("Cache export: '{}' -> RHI={} Layout={} (external={})",
               Name.GetNameString(), Entry->GetRHIID().GetHandle().GetIndex(),
               static_cast<int>(Entry->GetKnownLayout()),
               Entry->IsExternal());
            const auto& Info = Entry->GetCreateInfo();
            CacheSlot.InsertOrAssign(Name,
               FCachedTexture{
                  Entry->GetRHIID(),
                  Info.Width,
                  Info.Height,
                  Info.Format,
                  ERHIImageLayout::Undefined,
               });
         }

         RHI->Present(SwapChainID);
         RHI->EndFrame();

         if (MaxFrameRate > 0)
         {
            UInt32 const TargetMs = 1000 / MaxFrameRate;
            auto Elapsed = FramePacingClock.Now() - FrameStart;
            UInt32 ElapsedMs = Elapsed.Milliseconds();
            if (ElapsedMs + 2U < TargetMs)
            { FThread::Sleep(TargetMs - ElapsedMs - 2U); }
            while ((FramePacingClock.Now() - FrameStart).Milliseconds() < TargetMs)
            { /* spin last ~2ms for precise pacing */ }
         }
      }
   }

   // =================================================================
   // FGraphics — public API
   // =================================================================

   void FGraphics::
   UnregisterWindow(FWindow* I_Window)
   {
      if (!I_Window || !RHI) { return; }
      RHI->DestroySwapChain(I_Window);
      for (auto It = ManagedWindows.begin(); It != ManagedWindows.end(); ++It)
      {
         if (*It == I_Window) { ManagedWindows.Erase(It); break; }
      }
   }

   FRHISwapChainID FGraphics::
   RegisterHeadless()
   {
      FRHISwapChainID ID = RHI->CreateSwapChain(nullptr);
      if (ID != kInvalidSwapChainID)
      {
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
   Render(FWindow* I_Window, const FScene& I_Scene)
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
         ManagedWindows.PushBack(I_Window);
      }
      RHI->UpdateSwapChainMinimized(SwapChainID, I_Window->IsMinimized());

      UInt32 Prev = PendingDrawRenderTaskCount.FetchAdd(1, EMemoryOrder::Relaxed);
      if (Prev >= kMaxPendingDrawRenderTasks)
      {
         PendingDrawRenderTaskCount.FetchSub(1, EMemoryOrder::Relaxed);
         LOG_TRACE("Graphics render queue full, dropping draw intent.");
         return;
      }
      const UInt32 VW = static_cast<UInt32>(I_Window->GetWidth()  > 0 ? I_Window->GetWidth()  : 1);
      const UInt32 VH = static_cast<UInt32>(I_Window->GetHeight() > 0 ? I_Window->GetHeight() : 1);
      FRenderData data;
      FRenderView renderView;
      I_Scene.BuildRenderData(data);
      renderView = I_Scene.BuildRenderView();
      ChannelToGraphics.Send(TOptional<FRenderTask>{{SwapChainID, std::move(data), std::move(renderView), VW, VH}});
   }

   void FGraphics::
   Render(FRHISwapChainID I_SwapChainID, const FScene& I_Scene)
   {
      if (I_SwapChainID == kInvalidSwapChainID)
      {
         LOG_WARN("FGraphics::Render called with invalid SwapChainID.");
         return;
      }
      UInt32 Prev = PendingDrawRenderTaskCount.FetchAdd(1, EMemoryOrder::Relaxed);
      if (Prev >= kMaxPendingDrawRenderTasks)
      {
         PendingDrawRenderTaskCount.FetchSub(1, EMemoryOrder::Relaxed);
         LOG_TRACE("Graphics render queue full, dropping draw intent (id:{}).", I_SwapChainID);
         return;
      }
      FRenderData data;
      FRenderView renderView;
      I_Scene.BuildRenderData(data);
      renderView = I_Scene.BuildRenderView();
      ChannelToGraphics.Send(TOptional<FRenderTask>{{I_SwapChainID, std::move(data), std::move(renderView), 0, 0}});
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
