module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics;
#define VISERA_MODULE_NAME "Runtime.Graphics"
export import Visera.Runtime.Graphics.Scene;
export import Visera.Runtime.Graphics.Material;
export import Visera.Runtime.Graphics.RenderPipeline;
       import Visera.Runtime.Global;
       import Visera.Runtime.AssetHub;
       import Visera.Runtime.RHI;
       import Visera.Runtime.Window;
       import Visera.Core.Containers.Array;
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
   struct VISERA_RUNTIME_API FRenderTask
   {
      FRHISwapChainID   SwapChainID {kInvalidSwapChainID};
      FScene::FSnapshot Snapshot;
   };

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

      /** Register a pass factory. Lower priority values execute first. All pass sources (user, UI, debug) use this same API. */
      void
      RegisterPass(UInt32 I_Priority, FName I_Name,
                   TFunction<void(FRenderGraph&, const FRenderContext&)> I_Factory);

      [[nodiscard]] TSharedPtr<FMaterial>
      LoadMaterial(const FPath& I_MaterialFile);

      [[nodiscard]] const FRHI*
      GetRHI() const { return RHI.Get(); }

      ~FGraphics();

   private:

      /** Engine-internal pass: transitions BackBuffer to Present layout as the absolute last step. */
      class FPresentTransitionPass final : public IRGPass
      {
      public:
         explicit FPresentTransitionPass(FGraphicsID I_BackBuffer);

         void
         Execute(const FRenderGraph* I_Graph, FRHICommandList* I_CommandList) override;

         [[nodiscard]] const char*
         GetName() const override { return "Engine::PresentTransition"; }

         [[nodiscard]] EType
         GetType() const override { return EType::Render; }

      private:
         FGraphicsID BackBufferID;
      };

      /** Dedicated thread: take draw intent from channel → BeginFrame → call pass factories → PresentTransition → Compile → Execute → Present → EndFrame. */
      struct VISERA_RUNTIME_API FGraphicsThread
      {
      public:
         FGraphicsThread(
            FRHI*                                 I_RHI,
            TSPSCChannel<TOptional<FRenderTask>>& I_ChannelFromMain,
            TAtomic<UInt32>&                      I_PendingDrawRenderTaskCount,
            const TArray<FPassEntry>*             I_PassEntries,
            UInt32                                I_MaxFrameRate = 0);

         void
         Start();
         void
         RequestStop();
         void
         Join();

      private:
         void
         Run();

         FRHI*                                 RHI {nullptr};
         TSPSCChannel<TOptional<FRenderTask>>& ChannelFromMain;
         TAtomic<UInt32>&                      PendingDrawRenderTaskCount;
         UInt32                                MaxFrameRate {0};
         FHiResClock                           FramePacingClock;
         TUniquePtr<FThread>                   Thread;
         const TArray<FPassEntry>*             PassEntries {nullptr};
      };

      TSharedPtr<FAssetHub>                    AssetHub;
      TSharedPtr<FRHI>                         RHI;
      TSPSCChannel<TOptional<FRenderTask>>     ChannelToGraphics;
      TAtomic<UInt32>                          PendingDrawRenderTaskCount {0};
      TUniquePtr<FGraphicsThread>              GraphicsThread;
      TArray<FWindow*>                         ManagedWindows;
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
               LOG_INFO("Graphics: thread started (MaxFrameRate={}).", MaxFrameRate);
            }
            return True;
         }))
         { LOG_FATAL("Failed to bind bootstrap function!"); }

         if (!OnTerminate.TryBind([this]
         {
            for (auto* W : ManagedWindows)
            {
               auto Id = RHI->QuerySwapChainID(W);
               if (Id != kInvalidSwapChainID) { RHI->MarkSwapChainDestroyed(Id); }
            }
            for (auto Id : ManagedHeadlessIDs) { RHI->MarkSwapChainDestroyed(Id); }
            if (GraphicsThread)
            {
               GraphicsThread->RequestStop();
               GraphicsThread->Join();
               GraphicsThread.Reset();
            }
            if (RHI) { RHI->WaitIdle(); }
            for (auto* W : ManagedWindows)
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
   // FPresentTransitionPass
   // =================================================================

   FGraphics::FPresentTransitionPass::
   FPresentTransitionPass(FGraphicsID I_BackBuffer) : BackBufferID(I_BackBuffer) {}

   void FGraphics::FPresentTransitionPass::
   Execute(const FRenderGraph* I_Graph, FRHICommandList* I_CommandList)
   {
      const auto& BackBuffer = I_Graph->GetTexture(BackBufferID);
      if (BackBuffer.IsNull())
      {
         LOG_ERROR("PresentTransition: BackBuffer is null, skipping.");
         return;
      }
      I_CommandList->TransitionTexture(FRHIImageBarrier{
         .Image         = BackBuffer,
         .OldLayout     = ERHIImageLayout::TransferDst,
         .NewLayout     = ERHIImageLayout::Present,
         .MemoryBarrier = {
            .SourceStage  = ERHIPipelineStage::AllGraphics,
            .DestStage    = ERHIPipelineStage::BottomOfPipe,
            .SourceAccess = ERHIAccessFlag::ColorAttachmentWrite | ERHIAccessFlag::TransferWrite,
            .DestAccess   = ERHIAccessFlag::None,
         },
      });
   }

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
   {}

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
         auto FrameStart = FramePacingClock.Now();
         auto Received = ChannelFromMain.Receive();
         if (!Received.GetValue().HasValue())
         {
            LOG_DEBUG("({}) Graphics thread stopped.", RHI->GetRuntimeName());
            break;
         }
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

         const auto& Snapshot = RenderTask.Snapshot;
         const Bool bHasWindow = RHI->HasWindow(SwapChainID);

         if (bHasWindow && RHI->IsSwapChainDirty(SwapChainID))
         {
            RHI->WaitIdle();
            for (UInt32 Waited = 0; Waited < kMaxDirtyWaitMs && RHI->IsSwapChainDirty(SwapChainID); Waited += 1)
            { LOG_TRACE("Graphics thread: waiting for swapchain to be ready... ({}/{})", Waited, kMaxDirtyWaitMs); FThread::Sleep(1); }
         }

         FRHITextureID BackBuffer = RHI->BeginFrame(SwapChainID);
         if (bHasWindow && BackBuffer.IsNull()) { continue; }

         FRenderContext Ctx{
            .RHI         = RHI,
            .Scene       = &Snapshot,
            .SwapChainID = SwapChainID,
            .BackBuffer  = BackBuffer,
         };
         auto Graph = MakeUnique<FRenderGraph>(SwapChainID, BackBuffer);

         for (const auto& Entry : *PassEntries)
         {
            LOG_TRACE("Graphics thread: running pass factory '{}'.", Entry.Name.GetNameString());
            Entry.Factory(*Graph, Ctx);
         }

         if (bHasWindow && Graph->HasBackBuffer())
         {
            auto BackBufferID = Graph->GetBackBuffer();
            Graph->AddNode(
               MakeUnique<FPresentTransitionPass>(BackBufferID),
               {BackBufferID}, {BackBufferID});
         }

         auto CmdList = RHI->CreateCommandList();
         Graph->Compile(RHI)->Execute(&CmdList);
         RHI->Submit(std::move(CmdList));

         RHI->Present(SwapChainID);
         RHI->EndFrame();

         if (MaxFrameRate > 0)
         {
            auto Elapsed = FramePacingClock.Now() - FrameStart;
            UInt32 ElapsedMs = Elapsed.Milliseconds();
            UInt32 TargetMs  = 1000 / MaxFrameRate;
            if (ElapsedMs < TargetMs) { FThread::Sleep(TargetMs - ElapsedMs); }
         }
      }
   }

   // =================================================================
   // FGraphics — public API
   // =================================================================

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
      ChannelToGraphics.Send(TOptional<FRenderTask>{{SwapChainID, I_Scene.Snapshot()}});
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
      ChannelToGraphics.Send(TOptional<FRenderTask>{{I_SwapChainID, I_Scene.Snapshot()}});
   }

   TSharedPtr<FMaterial> FGraphics::
   LoadMaterial(const FPath& I_MaterialFile)
   {
      auto JSONOpt = FJSON::Load(I_MaterialFile);
      if (!JSONOpt.HasValue())
      { LOG_ERROR("LoadMaterial: failed to parse {}.", I_MaterialFile); return nullptr; }
      auto Material = FMaterial::Create(JSONOpt.GetValue(), AssetHub.Get(), RHI.Get());
      if (Material)
      { LOG_INFO("LoadMaterial: {} loaded successfully.", I_MaterialFile); }
      return Material;
   }

   FGraphics::~FGraphics()
   {
      PROFILING_ONLY_FIELD(LogRenderGraphCompileProfilingSummary();)
   }
}
