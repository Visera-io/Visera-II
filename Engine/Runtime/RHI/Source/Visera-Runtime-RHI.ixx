module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Common;
export import Visera.Runtime.RHI.Resource;
export import Visera.Runtime.RHI.CommandList;
export import Visera.Runtime.RHI.Registry;
       import Visera.Runtime.RHI.Vulkan;
       import Visera.Runtime.Window;
       import Visera.Runtime.Global;
       import Visera.Core.OS.Thread;
       import Visera.Core.Containers.Array;
       import Visera.Core.Containers.Map;
       import Visera.Core.Types.Pointer;
       import Visera.Core.Types.String;
       import Visera.Core.Delegate;
       import Visera.Core.Log;
       import Visera.Platform;
       import vulkan_hpp;

export namespace Visera
{
    class VISERA_RUNTIME_API FRHI : public IGlobalService
    {
        using FRHIDrawCalls     = FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>;
        using FRHITransferCalls = FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>;

    public:
        [[nodiscard]] Bool
        BeginFrame(FWindow* I_Window = nullptr);
        void
        EndFrame(FWindow* I_Window = nullptr);
        void
        Execute(FRHICommandList& I_CommandList);
        void
        Present(FWindow* I_Window = nullptr);
        void
        CreateSwapChain(FWindow* I_Window);
        void
        DestroySwapChain(FWindow* I_Window);
        void
        WaitIdle() const { Driver->WaitIdle(); }

        // Resource creation
        [[nodiscard]] FRHITextureID
        CreateTexture(FRHITextureCreateInfo&& I_Desc);
        [[nodiscard]] FRHIBufferID
        CreateBuffer(FRHIBufferCreateInfo&& I_Desc);
        [[nodiscard]] FRHISamplerID
        CreateSampler(FRHISamplerCreateInfo&& I_Desc);
        [[nodiscard]] FRHIDescriptorSetID
        CreateDescriptorSet(FRHIDescriptorSetCreateInfo&& I_Desc);

        // Low-level API
        [[nodiscard]] inline FVulkanDriver*
        GetDriver()
        {
            DEBUG_ONLY_FIELD(LOG_WARN("Accessed the RHI driver."));
            return Driver;
        }

    private:
        struct VISERA_RUNTIME_API FRHIThread
        {
            FRHI*   Owner { nullptr };
            FThread Thread;
            FEvent  WakeEvent;

            TSPSCQueue<TUniquePtr<FRHICommandList>> CommandListQueue;

            void Start(FRHI* I_Owner);
            void Run();
            void Stop();
            void Enqueue(TUniquePtr<FRHICommandList> I_CommandList); // Main Thread Use
        };

        FRHIThread     RHIThread;

        FVulkanDriver* Driver   {nullptr};
        FRHIRegistry*  Registry {nullptr};
        Bool           bOffScreenMode {False};

        FVulkanCommandPool<EVulkanQueueFamily::Graphics>
        GraphicsCommandPool;
        FVulkanCommandPool<EVulkanQueueFamily::Transfer>
        TransferCommandPool;

        struct FFrame
        {
            FVulkanFence      ExecuteFence;
            FVulkanSemaphore  SwapChainReadySemaphore;
            FVulkanSemaphore  RenderFinishedSemaphore;    // Offscreen only
            FRHIDrawCalls     DrawCalls;
            FVulkanSemaphore  TransferFinishedSemaphore;
            FRHITransferCalls TransferCalls;
        };
        struct FSwapChainContext
        {
            TArray<FFrame> Frames;
            TArray<FVulkanSemaphore> RenderFinishedSemaphores;
            UInt8 FrameIndex = 0;
            UInt8 LastSubmittedImageIndex = 0;
        };
        TMap<FWindow*, FSwapChainContext*> SwapChainContexts;
        TMap<FWindow*, TMulticastDelegate<FWindow*>::FHandle> ResizeHandles;
        FWindow* PrimaryWindow {nullptr};  // First swapchain, for parameterless overloads

        void
        SubscribeToWindowResize(FWindow* I_Window)
        {
            if (ResizeHandles.Contains(I_Window)) { return; }
            auto Handle = I_Window->OnResized.Subscribe([this](FWindow* I_Win)
            {
                if (I_Win->GetWidth() == 0 || I_Win->GetHeight() == 0)
                {
                    LOG_TRACE("({}) Skip SwapChain recreation while minimized ({}x{}).", GetRuntimeName(), I_Win->GetWidth(), I_Win->GetHeight());
                    return;
                }
                if (!SwapChainContexts.Contains(I_Win)) { return; }
                LOG_DEBUG("({}) Recreating SwapChain ({}x{}) for window (title:{}).",
                          GetRuntimeName(), I_Win->GetWidth(), I_Win->GetHeight(), I_Win->GetTitle());
                Driver->WaitIdle();
                if (auto It = SwapChainContexts.Find(I_Win); It != SwapChainContexts.end() && It->second)
                { delete It->second; }
                SwapChainContexts.Erase(I_Win);
                Driver->RecreateSwapChain(I_Win, I_Win->GetWidth(), I_Win->GetHeight());
                InitializeSwapChainContext(I_Win);
            });
            ResizeHandles.Insert(I_Window, Handle);
        }

        void
        UnsubscribeFromWindowResize(FWindow* I_Window)
        {
            if (auto It = ResizeHandles.Find(I_Window); It != ResizeHandles.end())
            {
                I_Window->OnResized.Unsubscribe(It->second);
                ResizeHandles.Erase(I_Window);
            }
        }

        void
        InitializeSwapChainContext(FWindow* I_Window)
        {
            auto* SC = Driver->GetSwapChain(I_Window);
            if (!SC) { return; }
            SwapChainContexts[I_Window] = new FSwapChainContext();
            auto* Ctx = SwapChainContexts[I_Window];
            Ctx->Frames.Resize(SC->Images.GetSize());
            Ctx->RenderFinishedSemaphores.Clear();
            for (UInt32 Idx = 0; Idx < SC->Images.GetSize(); ++Idx)
            {
                Ctx->RenderFinishedSemaphores.EmplaceBack(Driver->CreateSemaphore());
            }
            for (auto& Frame : Ctx->Frames)
            {
                Frame.ExecuteFence = Driver->CreateFence(True);
                Frame.SwapChainReadySemaphore = Driver->CreateSemaphore();
                Frame.DrawCalls = GraphicsCommandPool.CreateCommandBuffer(True);
                Frame.TransferFinishedSemaphore = Driver->CreateSemaphore();
                Frame.TransferCalls = TransferCommandPool.CreateCommandBuffer(True);
            }
            FRHIDrawCalls Cmd = GraphicsCommandPool.CreateCommandBuffer(True);
            Cmd.Begin();
            for (auto& Image : SC->Images)
            {
                Cmd.ConvertImageLayout(&Image,
                    vk::ImageLayout::ePresentSrcKHR,
                    EVulkanGraphicsStage::TopOfPipe,
                    EVulkanGraphicsAccess::None,
                    EVulkanGraphicsStage::BottomOfPipe,
                    EVulkanGraphicsAccess::None);
            }
            Cmd.End();
            FVulkanFence Fence = Driver->CreateFence(False);
            Driver->Submit(&Cmd, nullptr, nullptr, &Fence);
            if (!Fence.Wait())
            { LOG_FATAL("Failed to init RHI SwapChain!"); }
        }

        void
        InitializeOffscreenFrames()
        {
            FWindow* OffscreenKey = reinterpret_cast<FWindow*>(1); // Sentinel for offscreen
            SwapChainContexts[OffscreenKey] = new FSwapChainContext();
            auto* Ctx = SwapChainContexts[OffscreenKey];
            Ctx->Frames.Resize(1);
            for (auto& Frame : Ctx->Frames)
            {
                Frame.ExecuteFence = Driver->CreateFence(True);
                Frame.RenderFinishedSemaphore = Driver->CreateSemaphore();
                Frame.DrawCalls = GraphicsCommandPool.CreateCommandBuffer(True);
                Frame.TransferFinishedSemaphore = Driver->CreateSemaphore();
                Frame.TransferCalls = TransferCommandPool.CreateCommandBuffer(True);
            }
        }

    public:
        FRHI(FName I_Name, FServiceRegistry* I_Registry, const FJSON& I_Config)
            : IGlobalService(I_Name, I_Registry, I_Config)
        {
            Dependencies =
            {

            };

            bOffScreenMode = !I_Registry->Contains(EName::Window);
            
            if(!bOffScreenMode)
            {
                 Dependencies.Insert(EName::Window);
            }

            if (!OnBootstrap.TryBind([this]
            {
                TSharedPtr<FWindow> Window;
                if (!bOffScreenMode)
                {
                    if (auto WindowWeak = GetService<FWindow>(EName::Window); !WindowWeak.IsExpired())
                    {
                        Window = WindowWeak.Lock();
                    }
                    else
                    {
                        LOG_FATAL("Failed to get Window service!");
                        return False;
                    }

                    SubscribeToWindowResize(Window.Get());
                }
                vk::PresentModeKHR PresentMode = vk::PresentModeKHR::eFifo;
                UInt32 AppVersion = vk::makeVersion(1, 0, 0);
                {
                    FJSON RHIConfig = Config.GetObject("RHI");
                    FString PresentModeStr = RHIConfig.GetString("PresentMode", "VSync");
                    if (PresentModeStr == "Immediate") PresentMode = vk::PresentModeKHR::eImmediate;
                    else if (PresentModeStr == "Mailbox") PresentMode = vk::PresentModeKHR::eMailbox;
                    else if (PresentModeStr == "FIFO" || PresentModeStr == "VSync") PresentMode = vk::PresentModeKHR::eFifo;
                    if (RHIConfig.Contains("ApplicationVersionMajor") || RHIConfig.Contains("ApplicationVersionMinor") || RHIConfig.Contains("ApplicationVersionPatch"))
                    {
                        AppVersion = vk::makeVersion(
                            static_cast<UInt32>(RHIConfig.GetNumber("ApplicationVersionMajor", 1)),
                            static_cast<UInt32>(RHIConfig.GetNumber("ApplicationVersionMinor", 0)),
                            static_cast<UInt32>(RHIConfig.GetNumber("ApplicationVersionPatch", 0)));
                    }
                }
                Driver = new FVulkanDriver({.Window = Window, .SwapChainPresentMode = PresentMode, .bOffScreenMode = bOffScreenMode, .ApplicationName = GetRuntimeName(), .ApplicationVersion = AppVersion});

                if (Driver->GetDevice().GraphicsQueueFamilyIndex
                    !=
                    Driver->GetDevice().TransferQueueFamilyIndex)
                { LOG_WARN("({}) NOT support \"Queue Family Ownership Transfer\"!", GetRuntimeName()); }

                Registry = new FRHIRegistry(Driver);

                GraphicsCommandPool = Driver->CreateCommandPool<EVulkanQueueFamily::Graphics>(False);
                TransferCommandPool = Driver->CreateCommandPool<EVulkanQueueFamily::Transfer>(False);

                if (bOffScreenMode)
                { InitializeOffscreenFrames(); }
                else
                {
                    PrimaryWindow = Window.Get();
                    InitializeSwapChainContext(PrimaryWindow);
                }

                RHIThread.Start(this);

                return True;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                RHIThread.Stop();

                Driver->WaitIdle();
                for (auto& [W, H] : ResizeHandles)
                { W->OnResized.Unsubscribe(H); }
                ResizeHandles.Clear();
                for (auto& [W, Ctx] : SwapChainContexts) { if (Ctx) delete Ctx; }
                SwapChainContexts.Clear();
                GraphicsCommandPool = {};
                TransferCommandPool = {};
                delete Registry;
                delete Driver;
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        }
    private:
        template<typename T>
        [[nodiscard]] static const T&
        DecodePayload(const FCommandView& I_Cmd)
        {
            VISERA_ASSERT(I_Cmd.PayloadPtrAligned != nullptr);
            VISERA_ASSERT(I_Cmd.PayloadBytes == sizeof(T));
            return *reinterpret_cast<const T*>(I_Cmd.PayloadPtrAligned);
        }

        [[nodiscard]] FVulkanImage*
        GetVulkanImageChecked(FRHITextureHandle I_Handle) const
        {
            auto* Tex = Registry->Get(I_Handle);
            VISERA_ASSERT(Tex);
            return Tex->GetVulkanImage();
        }

        [[nodiscard]] FVulkanBuffer*
        GetVulkanBufferChecked(FRHIBufferHandle I_Handle) const
        {
            auto* Buf = Registry->Get(I_Handle);
            VISERA_ASSERT(Buf);
            return Buf->GetVulkanBuffer();
        }

        void ExecuteConvertImageLayout(FFrame& I_Frame, const FCommandView& I_Cmd);
        void ExecuteClearColorImage(FFrame& I_Frame, const FCommandView& I_Cmd);
        void ExecuteBlitImage(FFrame& I_Frame, const FCommandView& I_Cmd);
        void ExecuteBlitToSwapChain(FFrame& I_Frame, const FCommandView& I_Cmd, FWindow* I_TargetWindow);
        void ExecuteCopyBufferToImage(FFrame& I_Frame, const FCommandView& I_Cmd);
        void ExecuteWriteBuffer(FFrame& I_Frame, const FCommandView& I_Cmd);

        // Map a single vk::ImageLayout to the stage/access for barrier src or dst.
        static void MapGraphicsLayoutToBarrier(
            vk::ImageLayout        I_Layout,
            EVulkanGraphicsStage*  IO_Stage,
            EVulkanGraphicsAccess* IO_Access);

        // Infer src/dst from old and new layout (map both, then use as barrier).
        void InferGraphicsBarrier(
            vk::ImageLayout        I_OldLayout,
            vk::ImageLayout        I_NewLayout,
            EVulkanGraphicsStage*  IO_SrcStage,
            EVulkanGraphicsAccess* IO_SrcAccess,
            EVulkanGraphicsStage*  IO_DstStage,
            EVulkanGraphicsAccess* IO_DstAccess) const;

        static void MapTransferLayoutToBarrier(
            vk::ImageLayout        I_Layout,
            EVulkanTransferStage*  IO_Stage,
            EVulkanTransferAccess* IO_Access);

        void InferTransferBarrier(
            vk::ImageLayout        I_OldLayout,
            vk::ImageLayout        I_NewLayout,
            EVulkanTransferStage*  IO_SrcStage,
            EVulkanTransferAccess* IO_SrcAccess,
            EVulkanTransferStage*  IO_DstStage,
            EVulkanTransferAccess* IO_DstAccess) const;
    };

    void FRHI::
    CreateSwapChain(FWindow* I_Window)
    {
        if (bOffScreenMode) { return; }
        if (SwapChainContexts.Contains(I_Window)) { return; }
        Driver->CreateSwapChain(I_Window);
        SubscribeToWindowResize(I_Window);
        InitializeSwapChainContext(I_Window);
    }

    void FRHI::
    DestroySwapChain(FWindow* I_Window)
    {
        if (bOffScreenMode) { return; }
        UnsubscribeFromWindowResize(I_Window);
        if (auto It = SwapChainContexts.Find(I_Window); It != SwapChainContexts.end() && It->second)
        { delete It->second; }
        SwapChainContexts.Erase(I_Window);
        Driver->DestroySwapChain(I_Window);
    }

    void FRHI::FRHIThread::
    Start(FRHI* I_Owner)
    {
        VISERA_ASSERT(I_Owner && !Owner);
        Owner = I_Owner;

        Thread.Start([this]
        {
            LOG_DEBUG("({}) RHI thread started.", Owner->GetRuntimeName());
            FPlatform::SetCurrentThreadName("RHI");
            try         { Run(); }
            catch (...) { LOG_FATAL("RHI thread crashed by exception."); }
        });
    }

    void FRHI::FRHIThread::
    Run()
    {
        while (!Thread.ShouldStop())
        {
            Bool bExecuted = False;
            do
            {
                if (auto R = CommandListQueue.Dequeue(); R.HasValue())
                {
                    bExecuted = True;
                    Owner->Execute(*std::move(R).GetValue());
                }
                else break;
            }while (True);

            if (!bExecuted) { WakeEvent.Wait(); }
        }

        if (Owner) { Owner->WaitIdle(); }
    }

    void FRHI::FRHIThread::
    Stop()
    {
        Thread.RequestStop();
        WakeEvent.Trigger();
        Thread.Join();
        auto RuntimeName = Owner ? Owner->GetRuntimeName() : FString("Unknown");
        Owner = nullptr;
        LOG_DEBUG("({}) RHI thread stopped.", RuntimeName);
    }

    void FRHI::FRHIThread::
    Enqueue(TUniquePtr<FRHICommandList> I_CommandList)
    {
        CommandListQueue.Enqueue(std::move(I_CommandList));
        WakeEvent.Trigger();
    }

    Bool FRHI::
    BeginFrame(FWindow* I_Window)
    {
        FWindow* Win = I_Window ? I_Window : PrimaryWindow;
        if (!Win && bOffScreenMode) { Win = reinterpret_cast<FWindow*>(1); }
        if (!Win) { return False; }
        auto It = SwapChainContexts.Find(Win);
        if (It == SwapChainContexts.end() || !It->second) { return False; }
        auto* Ctx = It->second;

        FFrame& CurrentFrame = Ctx->Frames[Ctx->FrameIndex];
        if (!CurrentFrame.ExecuteFence.Wait()) { return False; }

        Registry->SetCurrentRetirementFence(&CurrentFrame.ExecuteFence);
        Registry->CollectGarbage();
        Bool bAcquired = bOffScreenMode ? True : Driver->WaitNextFrame(Win, &CurrentFrame.SwapChainReadySemaphore);
        if (bAcquired)
        {
            if (!CurrentFrame.ExecuteFence.Reset())
            {
                LOG_ERROR("({}) Failed to reset the Fence!", GetRuntimeName());
                return False;
            }
        }
        else
        {
            LOG_TRACE("({}) Failed to begin new frame!", GetRuntimeName());
            return False;
        }
        CurrentFrame.TransferCalls.Reset();
        CurrentFrame.TransferCalls.Begin();
        CurrentFrame.DrawCalls.Reset();
        CurrentFrame.DrawCalls.Begin();

        return True;
    }

    void FRHI::
    EndFrame(FWindow* I_Window)
    {
        FWindow* Win = I_Window ? I_Window : PrimaryWindow;
        if (!Win && bOffScreenMode) { Win = reinterpret_cast<FWindow*>(1); }
        if (!Win) { return; }
        auto It = SwapChainContexts.Find(Win);
        if (It == SwapChainContexts.end() || !It->second) { return; }
        auto* Ctx = It->second;

        FFrame& CurrentFrame = Ctx->Frames[Ctx->FrameIndex];
        CurrentFrame.TransferCalls.End();
        CurrentFrame.DrawCalls.End();

        FVulkanSemaphore* TransferWait = bOffScreenMode ? nullptr : &CurrentFrame.SwapChainReadySemaphore;
        Driver->Submit(&CurrentFrame.TransferCalls,
            TransferWait,
            &CurrentFrame.TransferFinishedSemaphore,
            nullptr);

        if (bOffScreenMode)
        {
            Driver->Submit(&CurrentFrame.DrawCalls,
                &CurrentFrame.TransferFinishedSemaphore,
                &CurrentFrame.RenderFinishedSemaphore,
                &CurrentFrame.ExecuteFence);
        }
        else
        {
            auto* SC = Driver->GetSwapChain(Win);
            if (SC)
            {
                const UInt8 ImageIndex = SC->Cursor;
                Driver->Submit(&CurrentFrame.DrawCalls,
                    &CurrentFrame.TransferFinishedSemaphore,
                    &Ctx->RenderFinishedSemaphores[ImageIndex],
                    &CurrentFrame.ExecuteFence);
                Ctx->LastSubmittedImageIndex = ImageIndex;
            }
        }

        Registry->ClearGarbage();
        Ctx->FrameIndex = (Ctx->FrameIndex + 1) % Ctx->Frames.GetSize();
    }

    void FRHI::
    Execute(FRHICommandList& I_CommandList)
    {
        FWindow* Win = static_cast<FWindow*>(I_CommandList.TargetSwapChain);
        if (!Win && bOffScreenMode) { Win = reinterpret_cast<FWindow*>(1); }
        if (!Win) { Win = PrimaryWindow; }
        if (!Win) { return; }
        auto It = SwapChainContexts.Find(Win);
        if (It == SwapChainContexts.end() || !It->second) { return; }
        auto* Ctx = It->second;
        FFrame& Frame = Ctx->Frames[Ctx->FrameIndex];
        VISERA_ASSERT(Frame.DrawCalls.IsRecording());
        VISERA_ASSERT(Frame.TransferCalls.IsRecording());

        for (auto Command : I_CommandList)
        {
            if (Command.PayloadPtrAligned == nullptr) { continue; }

            switch (Command.Type)
            {
            case ECommandType::ConvertImageLayout:  ExecuteConvertImageLayout(Frame, Command); break;
            case ECommandType::ClearColorImage:     ExecuteClearColorImage(Frame, Command); break;
            case ECommandType::BlitImage:           ExecuteBlitImage(Frame, Command); break;
            case ECommandType::BlitToSwapChain:     ExecuteBlitToSwapChain(Frame, Command, Win); break;
            case ECommandType::CopyBufferToImage:   ExecuteCopyBufferToImage(Frame, Command); break;
            case ECommandType::WriteBuffer:         ExecuteWriteBuffer(Frame, Command); break;
            default: LOG_ERROR("({}) Unknown Command Type: {}", GetRuntimeName(), static_cast<UInt16>(Command.Type)); break;
            }
        }
    }

    void FRHI::
    ExecuteConvertImageLayout(FFrame& I_Frame, const FCommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FConvertImageLayout>(I_Cmd);
        auto* Img = GetVulkanImageChecked(Payload.Image);
        vk::ImageLayout OldLayout = Img->GetLayout();
        vk::ImageLayout NewLayout = TypeCast(Payload.NewLayout);
        EVulkanGraphicsStage  SrcStage{},  DstStage{};
        EVulkanGraphicsAccess SrcAccess{}, DstAccess{};
        InferGraphicsBarrier(OldLayout, NewLayout, &SrcStage, &SrcAccess, &DstStage, &DstAccess);
        I_Frame.DrawCalls.ConvertImageLayout(Img, NewLayout, SrcStage, SrcAccess, DstStage, DstAccess);
    }

    void FRHI::
    ExecuteClearColorImage(FFrame& I_Frame, const FCommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FClearColorImage>(I_Cmd);
        auto* Img = GetVulkanImageChecked(Payload.Image);
        I_Frame.DrawCalls.ClearColorImage(Img, {
            Payload.ClearColor.R,
            Payload.ClearColor.G,
            Payload.ClearColor.B,
            Payload.ClearColor.A,
        });
    }

    void FRHI::
    ExecuteBlitImage(FFrame& I_Frame, const FCommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FBlitImage>(I_Cmd);
        auto* SrcImg = GetVulkanImageChecked(Payload.SrcImage);
        auto* DstImg = GetVulkanImageChecked(Payload.DstImage);
        I_Frame.DrawCalls.BlitImage(SrcImg, DstImg, TypeCast(Payload.Filter));
    }

    void FRHI::
    ExecuteBlitToSwapChain(FFrame& I_Frame, const FCommandView& I_Cmd, FWindow* I_TargetWindow)
    {
        if (bOffScreenMode || !I_TargetWindow) { return; }
        auto* SC = Driver->GetSwapChain(I_TargetWindow);
        if (!SC) { return; }
        const auto& Payload = DecodePayload<FRHICommandList::FBlitToSwapChain>(I_Cmd);
        auto* Texture        = GetVulkanImageChecked(Payload.Image);
        auto* SwapChainImage = SC->GetCurrentImage();
        {
            vk::ImageLayout OldLayout = SwapChainImage->GetLayout();
            vk::ImageLayout NewLayout = TypeCast(ERHIImageLayout::TransferDst);
            EVulkanGraphicsStage  SrcStage{},  DstStage{};
            EVulkanGraphicsAccess SrcAccess{}, DstAccess{};
            InferGraphicsBarrier(OldLayout, NewLayout, &SrcStage, &SrcAccess, &DstStage, &DstAccess);
            I_Frame.DrawCalls.ConvertImageLayout(SwapChainImage, NewLayout, SrcStage, SrcAccess, DstStage, DstAccess);
        }
        I_Frame.DrawCalls.BlitImage(Texture, SwapChainImage, TypeCast(Payload.Filter));
        {
            vk::ImageLayout OldLayout = SwapChainImage->GetLayout();
            vk::ImageLayout NewLayout = TypeCast(ERHIImageLayout::Present);
            EVulkanGraphicsStage  SrcStage{},  DstStage{};
            EVulkanGraphicsAccess SrcAccess{}, DstAccess{};
            InferGraphicsBarrier(OldLayout, NewLayout, &SrcStage, &SrcAccess, &DstStage, &DstAccess);
            I_Frame.DrawCalls.ConvertImageLayout(SwapChainImage, NewLayout, SrcStage, SrcAccess, DstStage, DstAccess);
        }
    }

    void FRHI::
    ExecuteCopyBufferToImage(FFrame& I_Frame, const FCommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FCopyBufferToImage>(I_Cmd);
        auto* VulkanBuffer = GetVulkanBufferChecked(Payload.Buffer);
        auto* VulkanImage  = GetVulkanImageChecked(Payload.Image);
        vk::ImageLayout OldLayout   = VulkanImage->GetLayout();
        vk::ImageLayout TransferDst = TypeCast(ERHIImageLayout::TransferDst);
        {
            EVulkanTransferStage  SrcStage{}, DstStage{};
            EVulkanTransferAccess SrcAccess{}, DstAccess{};
            InferTransferBarrier(OldLayout, TransferDst, &SrcStage, &SrcAccess, &DstStage, &DstAccess);
            I_Frame.TransferCalls.ConvertImageLayout(VulkanImage, TransferDst, SrcStage, SrcAccess, DstStage, DstAccess);
        }
        I_Frame.TransferCalls.CopyBufferToImage(VulkanBuffer, VulkanImage);
        {
            EVulkanTransferStage  SrcStage{}, DstStage{};
            EVulkanTransferAccess SrcAccess{}, DstAccess{};
            InferTransferBarrier(TransferDst, OldLayout, &SrcStage, &SrcAccess, &DstStage, &DstAccess);
            I_Frame.TransferCalls.ConvertImageLayout(VulkanImage, OldLayout, SrcStage, SrcAccess, DstStage, DstAccess);
        }
    }

    void FRHI::
    ExecuteWriteBuffer(FFrame& I_Frame, const FCommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FWriteBuffer>(I_Cmd);
        auto* TargetBuffer  = GetVulkanBufferChecked(Payload.TargetBuffer);
        auto* StagingBuffer = GetVulkanBufferChecked(Payload.StagingBuffer);
        I_Frame.TransferCalls.CopyBuffer(StagingBuffer, TargetBuffer);
    }

    void FRHI::
    Present(FWindow* I_Window)
    {
        if (bOffScreenMode) { return; }
        FWindow* Window = I_Window ? I_Window : PrimaryWindow;
        if (!Window) { return; }
        auto It = SwapChainContexts.Find(Window);
        if (It == SwapChainContexts.end() || !It->second) { return; }
        auto* Ctx = It->second;
        if (!Driver->Present(Window, &Ctx->RenderFinishedSemaphores[Ctx->LastSubmittedImageIndex]))
        {
            LOG_DEBUG("({}) Failed to present frame!", GetRuntimeName());
        }
    }

    FRHITextureID FRHI::
    CreateTexture(FRHITextureCreateInfo&& I_Desc)
    {
        const auto W = I_Desc.Width, H = I_Desc.Height, D = I_Desc.Depth;
        const auto Fmt = I_Desc.Format;
        auto ID = Registry->Register(std::move(I_Desc));
        LOG_DEBUG("({}) CreateTexture: {}x{}x{} {} -> {}", GetRuntimeName(), W, H, D, Fmt, ID.GetHandle());
        return ID;
    }

    FRHIBufferID FRHI::
    CreateBuffer(FRHIBufferCreateInfo&& I_Desc)
    {
        const auto Size = I_Desc.Size;
        auto ID = Registry->Register(std::move(I_Desc));
        LOG_DEBUG("({}) CreateBuffer: {} bytes -> {}", GetRuntimeName(), Size, ID.GetHandle());
        return ID;
    }

    FRHISamplerID FRHI::
    CreateSampler(FRHISamplerCreateInfo&& I_Desc)
    {
        const auto Type = I_Desc.Type;
        const auto Addr = I_Desc.AddressMode;
        auto ID = Registry->Register(std::move(I_Desc));
        LOG_DEBUG("({}) CreateSampler: {} {} -> {}", GetRuntimeName(), Type, Addr, ID.GetHandle());
        return ID;
    }

    FRHIDescriptorSetID FRHI::
    CreateDescriptorSet(FRHIDescriptorSetCreateInfo&& I_Desc)
    {
        const auto BindingCount = I_Desc.Bindings.GetSize();
        auto ID = Registry->Register(std::move(I_Desc));
        LOG_DEBUG("({}) CreateDescriptorSet: {} bindings -> {}",
                  GetRuntimeName(), BindingCount, ID.GetHandle());
        return ID;
    }

    void FRHI::
    MapGraphicsLayoutToBarrier(
        vk::ImageLayout         I_Layout,
        EVulkanGraphicsStage*  IO_Stage,
        EVulkanGraphicsAccess* IO_Access)
    {
        VISERA_ASSERT(IO_Stage && IO_Access);

        switch (I_Layout)
        {
        case vk::ImageLayout::eUndefined:
            *IO_Stage  = EVulkanGraphicsStage::TopOfPipe;
            *IO_Access = EVulkanGraphicsAccess::None;
            return;

        case vk::ImageLayout::eTransferDstOptimal:
            *IO_Stage  = EVulkanGraphicsStage::Transfer;
            *IO_Access = EVulkanGraphicsAccess::TransferWrite;
            return;

        case vk::ImageLayout::eTransferSrcOptimal:
            *IO_Stage  = EVulkanGraphicsStage::Transfer;
            *IO_Access = EVulkanGraphicsAccess::TransferRead;
            return;

        case vk::ImageLayout::eColorAttachmentOptimal:
            *IO_Stage  = EVulkanGraphicsStage::ColorAttachmentOutput;
            *IO_Access = EVulkanGraphicsAccess::ColorAttachmentWrite;
            return;

        case vk::ImageLayout::eDepthStencilAttachmentOptimal:
            *IO_Stage  = EVulkanGraphicsStage::EarlyFragmentTests | EVulkanGraphicsStage::LateFragmentTests;
            *IO_Access = EVulkanGraphicsAccess::DepthStencilAttachmentWrite;
            return;

        case vk::ImageLayout::eShaderReadOnlyOptimal:
            *IO_Stage  = EVulkanGraphicsStage::FragmentShader;
            *IO_Access = EVulkanGraphicsAccess::ShaderSampledRead;
            return;

        case vk::ImageLayout::eGeneral:
            *IO_Stage  = EVulkanGraphicsStage::FragmentShader;
            *IO_Access = EVulkanGraphicsAccess::ShaderRead | EVulkanGraphicsAccess::ShaderWrite;
            return;

        case vk::ImageLayout::ePresentSrcKHR:
            // For src side: treat present as "no access".
            *IO_Stage  = EVulkanGraphicsStage::BottomOfPipe;
            *IO_Access = EVulkanGraphicsAccess::None;
            return;

        default:
            // Bring-up safe fallback.
            *IO_Stage  = EVulkanGraphicsStage::AllCommands;
            *IO_Access = EVulkanGraphicsAccess::MemoryRead | EVulkanGraphicsAccess::MemoryWrite;
            return;
        }
    }

    void FRHI::
    InferGraphicsBarrier(
        vk::ImageLayout        I_OldLayout,
        vk::ImageLayout        I_NewLayout,
        EVulkanGraphicsStage*  IO_SrcStage,
        EVulkanGraphicsAccess* IO_SrcAccess,
        EVulkanGraphicsStage*  IO_DstStage,
        EVulkanGraphicsAccess* IO_DstAccess) const
    {
        VISERA_ASSERT(IO_SrcStage && IO_SrcAccess && IO_DstStage && IO_DstAccess);

        MapGraphicsLayoutToBarrier(I_OldLayout, IO_SrcStage, IO_SrcAccess);
        MapGraphicsLayoutToBarrier(I_NewLayout, IO_DstStage, IO_DstAccess);

        // If src was read-only, it is still fine. If src was unknown, fallback already covered.
    }

    void FRHI::
    MapTransferLayoutToBarrier(
        vk::ImageLayout        I_Layout,
        EVulkanTransferStage*  IO_Stage,
        EVulkanTransferAccess* IO_Access)
    {
        VISERA_ASSERT(IO_Stage && IO_Access);

        switch (I_Layout)
        {
        case vk::ImageLayout::eUndefined:
            *IO_Stage  = EVulkanTransferStage::TopOfPipe;
            *IO_Access = EVulkanTransferAccess::None;
            return;

        case vk::ImageLayout::eTransferDstOptimal:
            *IO_Stage  = EVulkanTransferStage::Transfer;
            *IO_Access = EVulkanTransferAccess::TransferWrite;
            return;

        case vk::ImageLayout::eTransferSrcOptimal:
            *IO_Stage  = EVulkanTransferStage::Transfer;
            *IO_Access = EVulkanTransferAccess::TransferRead;
            return;

        case vk::ImageLayout::ePresentSrcKHR:
            *IO_Stage  = EVulkanTransferStage::BottomOfPipe;
            *IO_Access = EVulkanTransferAccess::None;
            return;

        default:
            // Transfer queue mainly cares about TransferSrc/Dst; other layouts fallback.
            *IO_Stage  = EVulkanTransferStage::AllCommands;
            *IO_Access = EVulkanTransferAccess::MemoryRead;
            return;
        }
    }

    void FRHI::
    InferTransferBarrier(
        vk::ImageLayout        I_OldLayout,
        vk::ImageLayout        I_NewLayout,
        EVulkanTransferStage*  IO_SrcStage,
        EVulkanTransferAccess* IO_SrcAccess,
        EVulkanTransferStage*  IO_DstStage,
        EVulkanTransferAccess* IO_DstAccess) const
    {
        VISERA_ASSERT(IO_SrcStage && IO_SrcAccess && IO_DstStage && IO_DstAccess);

        MapTransferLayoutToBarrier(I_OldLayout, IO_SrcStage, IO_SrcAccess);
        MapTransferLayoutToBarrier(I_NewLayout, IO_DstStage, IO_DstAccess);
    }
}
