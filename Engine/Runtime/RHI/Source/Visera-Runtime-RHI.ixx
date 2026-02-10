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
       import Visera.Core.OS.Thread.Queue.SPSC;
       import Visera.Core.Containers.Array;
       import Visera.Core.Types.Pointer;
       import Visera.Core.Types.String;
       import Visera.Core.Delegate;
       import Visera.Core.Log;
       import vulkan_hpp;

export namespace Visera
{
    class VISERA_RUNTIME_API FRHI : public IGlobalService
    {
        using FRHIDrawCalls     = FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>;
        using FRHITransferCalls = FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>;
    public:
        [[nodiscard]] Bool
        BeginFrame();
        void
        EndFrame();
        void
        Submit(FRHICommandList&& I_CommandList);
        void
        Present();

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
        [[nodiscard]] inline const FVulkanDriver*
        GetDriver() const
        {
            DEBUG_ONLY_FIELD(LOG_WARN("Accessed the RHI driver."));
            return Driver;
        }

    private:
        FVulkanDriver* Driver   {nullptr};
        FRHIRegistry*  Registry {nullptr};



        FVulkanCommandPool<EVulkanQueueFamily::Graphics>
        GraphicsCommandPool;
        FVulkanCommandPool<EVulkanQueueFamily::Transfer>
        TransferCommandPool;

        struct FFrame
        {
            FVulkanFence      SubmitFence;
#if !defined(VISERA_OFFSCREEN_MODE)
            FVulkanSemaphore  SwapChainReadySemaphore;
#endif
#if defined(VISERA_OFFSCREEN_MODE)
            FVulkanSemaphore  RenderFinishedSemaphore;
#endif
            FRHIDrawCalls     DrawCalls;

            FVulkanSemaphore  TransferFinishedSemaphore;
            FRHITransferCalls TransferCalls;
        };
        TArray<FFrame> InFlightFrames;
        UInt8          FrameIndex = 0;
        UInt8          LastSubmittedFrameIndex = 0; // Frame index that was last submitted in EndFrame()
#if !defined(VISERA_OFFSCREEN_MODE)
        // Per-swapchain-image semaphores for present. Required for correct semaphore reuse per Vulkan spec.
        TArray<FVulkanSemaphore> RenderFinishedSemaphores;
        UInt8                    LastSubmittedImageIndex = 0;
#endif

        void
        InitializeSwapChain()
        {
#if !defined(VISERA_OFFSCREEN_MODE)
            InFlightFrames.Resize(Driver->GetSwapChain().Images.GetSize());
            RenderFinishedSemaphores.Clear();
            for (UInt32 Idx = 0; Idx < Driver->GetSwapChain().Images.GetSize(); ++Idx)
            {
                RenderFinishedSemaphores.EmplaceBack(Driver->CreateSemaphore());
            }
            for (auto& Frame : InFlightFrames)
            {
                Frame.SubmitFence = Driver->CreateFence(True);

                Frame.SwapChainReadySemaphore = Driver->CreateSemaphore();

                Frame.DrawCalls = GraphicsCommandPool.CreateCommandBuffer(True);

                Frame.TransferFinishedSemaphore = Driver->CreateSemaphore();
                Frame.TransferCalls = TransferCommandPool.CreateCommandBuffer(True);
            }
            FRHIDrawCalls Cmd = GraphicsCommandPool.CreateCommandBuffer(True);
            Cmd.Begin();
            {
                for (auto& Image : Driver->GetSwapChain().Images)
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
                {
                    LOG_FATAL("Failed to init RHI SwapChain!");
                }
#else
            InFlightFrames.Resize(1);
            for (auto& Frame : InFlightFrames)
            {
                Frame.SubmitFence = Driver->CreateFence(True);

                Frame.RenderFinishedSemaphore = Driver->CreateSemaphore();
                Frame.DrawCalls = GraphicsCommandPool.CreateCommandBuffer(True);

                Frame.TransferFinishedSemaphore = Driver->CreateSemaphore();
                Frame.TransferCalls = TransferCommandPool.CreateCommandBuffer(True);
            }
#endif
        }

    public:
        FRHI(FName I_Name, FServiceRegistry* I_Registry, const FJSON& I_Config)
            : IGlobalService(I_Name, I_Registry, I_Config)
        {
            Dependencies =
            {
#if !defined(VISERA_OFFSCREEN_MODE)
                EName::Window,
#endif
            };

            if (!OnBootstrap.TryBind([this]
            {
                TSharedPtr<FWindow> Window;
#if !defined(VISERA_OFFSCREEN_MODE)
                if (auto WindowWeak = GetService<FWindow>(EName::Window); !WindowWeak.IsExpired())
                {
                    Window = WindowWeak.Lock();
                }
                else
                {
                    LOG_FATAL("Failed to get Window service!");
                    return False;
                }

                Window->OnResized.Subscribe([this]
                (UInt32 I_NewWidth, UInt32 I_NewHeight)
                {
                    // When the window is minimized, framebuffer size can become 0x0.
                    // Vulkan forbids creating a swapchain with zero extent, so skip until restored.
                    if (I_NewWidth == 0 || I_NewHeight == 0)
                    {
                        LOG_TRACE("Skip SwapChain recreation while minimized ({}x{}).",
                                  I_NewWidth, I_NewHeight);
                        return;
                    }

                    if (auto WindowWeak = GetService<FWindow>(EName::Window); !WindowWeak.IsExpired())
                    {
                        auto Window = WindowWeak.Lock();
                        LOG_DEBUG("Recreating SwapChain ({}x{}).", I_NewWidth, I_NewHeight);
                        Driver->RecreateSwapChain(I_NewWidth, I_NewHeight, Window);
                        InitializeSwapChain();
                    }
                    else { LOG_ERROR("Failed to get Window!"); }
                });
#endif
                vk::PresentModeKHR PresentMode = vk::PresentModeKHR::eFifo;
                {
                    FJSON RHIConfig = Config.GetObject("RHI");
                    FString PresentModeStr = RHIConfig.GetString("PresentMode", "VSync");
                    if (PresentModeStr == "Immediate") PresentMode = vk::PresentModeKHR::eImmediate;
                    else if (PresentModeStr == "Mailbox") PresentMode = vk::PresentModeKHR::eMailbox;
                    else if (PresentModeStr == "FIFO" || PresentModeStr == "VSync") PresentMode = vk::PresentModeKHR::eFifo;
                }
                Driver = new FVulkanDriver({.Window = Window, .SwapChainPresentMode = PresentMode});

                if (Driver->GetDevice().GraphicsQueueFamilyIndex
                    !=
                    Driver->GetDevice().TransferQueueFamilyIndex)
                { LOG_WARN("NOT support \"Queue Family Ownership Transfer\"!"); }

                Registry = new FRHIRegistry(Driver);

                // Command Pools
                GraphicsCommandPool = Driver->CreateCommandPool<EVulkanQueueFamily::Graphics>
                (
                    False
                );
                TransferCommandPool = Driver->CreateCommandPool<EVulkanQueueFamily::Transfer>
                (
                    False
                );

                InitializeSwapChain(); // noop in offscreen mode

                return True;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                Driver->WaitIdle();
#if !defined(VISERA_OFFSCREEN_MODE)
                RenderFinishedSemaphores.Clear();
#endif
                InFlightFrames.Clear();
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
        void ExecuteBlitToSwapChain(FFrame& I_Frame, const FCommandView& I_Cmd);
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

    Bool FRHI::
    BeginFrame()
    {
        FFrame& CurrentFrame = InFlightFrames[FrameIndex];

        if (!CurrentFrame.SubmitFence.Wait()) { return False; }

        Registry->SetCurrentRetirementFence(&CurrentFrame.SubmitFence);
        Registry->CollectGarbage();
#if !defined(VISERA_OFFSCREEN_MODE)
        if (Driver->WaitNextFrame(&CurrentFrame.SwapChainReadySemaphore))
#else
        if (True)
#endif
        {
            if (!CurrentFrame.SubmitFence.Reset())
            {
                LOG_ERROR("Failed to reset the Fence!");
                return False;
            }
        }
#if !defined(VISERA_OFFSCREEN_MODE)
        else
        {
            LOG_TRACE("Failed to begin new frame!");
            return False; // Potential reasons: minimized window ...
        }
#endif
        CurrentFrame.TransferCalls.Reset();
        CurrentFrame.TransferCalls.Begin();
        CurrentFrame.DrawCalls.Reset();
        CurrentFrame.DrawCalls.Begin();

        return True;
    }

    void FRHI::
    EndFrame()
    {
        FFrame& CurrentFrame = InFlightFrames[FrameIndex];

        CurrentFrame.TransferCalls.End();
        CurrentFrame.DrawCalls.End();

        Driver->Submit(&CurrentFrame.TransferCalls,
#if !defined(VISERA_OFFSCREEN_MODE)
        &CurrentFrame.SwapChainReadySemaphore,
#else
        nullptr,
#endif
        &CurrentFrame.TransferFinishedSemaphore,
        nullptr);

#if !defined(VISERA_OFFSCREEN_MODE)
        const UInt8 ImageIndex = Driver->GetSwapChain().Cursor;
        Driver->Submit(&CurrentFrame.DrawCalls,
            &CurrentFrame.TransferFinishedSemaphore,
            &RenderFinishedSemaphores[ImageIndex],
            &CurrentFrame.SubmitFence);
        LastSubmittedImageIndex = ImageIndex;
#else
        Driver->Submit(&CurrentFrame.DrawCalls,
            &CurrentFrame.TransferFinishedSemaphore,
            &CurrentFrame.RenderFinishedSemaphore,
            &CurrentFrame.SubmitFence);
#endif

        Registry->ClearGarbage();

        // Record the frame index that was just submitted before incrementing
        LastSubmittedFrameIndex = FrameIndex;
        FrameIndex = (FrameIndex + 1) % InFlightFrames.GetSize();
    }

    void FRHI::
    Submit(FRHICommandList&& I_CommandList)
    {
        FFrame& Frame = InFlightFrames[FrameIndex];
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
            case ECommandType::BlitToSwapChain:     ExecuteBlitToSwapChain(Frame, Command); break;
            case ECommandType::CopyBufferToImage:   ExecuteCopyBufferToImage(Frame, Command); break;
            case ECommandType::WriteBuffer:         ExecuteWriteBuffer(Frame, Command); break;
            default: LOG_ERROR("Unknown Command Type: {}", static_cast<UInt16>(Command.Type)); break;
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
    ExecuteBlitToSwapChain(FFrame& I_Frame, const FCommandView& I_Cmd)
    {
#if !defined(VISERA_OFFSCREEN_MODE)
        const auto& Payload = DecodePayload<FRHICommandList::FBlitToSwapChain>(I_Cmd);
        auto* Texture        = GetVulkanImageChecked(Payload.Image);
        auto* SwapChainImage = Driver->GetSwapChain().GetCurrentImage();
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
#endif
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
    Present()
    {
#if !defined(VISERA_OFFSCREEN_MODE)
        // Use image-indexed semaphore for correct swapchain semaphore reuse per Vulkan spec
        if (!Driver->Present(&RenderFinishedSemaphores[LastSubmittedImageIndex]))
        {
            LOG_DEBUG("Failed to present frame {}!", LastSubmittedFrameIndex);
        }
#else
        // No-op in offscreen mode
#endif
    }

    FRHITextureID FRHI::
    CreateTexture(FRHITextureCreateInfo&& I_Desc)
    {
        const auto W = I_Desc.Width, H = I_Desc.Height, D = I_Desc.Depth;
        const auto Fmt = I_Desc.Format;
        auto ID = Registry->Register(std::move(I_Desc));
        LOG_DEBUG("CreateTexture: {}x{}x{} {} -> {}", W, H, D, Fmt, ID.GetHandle());
        return ID;
    }

    FRHIBufferID FRHI::
    CreateBuffer(FRHIBufferCreateInfo&& I_Desc)
    {
        const auto Size = I_Desc.Size;
        auto ID = Registry->Register(std::move(I_Desc));
        LOG_DEBUG("CreateBuffer: {} bytes -> {}", Size, ID.GetHandle());
        return ID;
    }

    FRHISamplerID FRHI::
    CreateSampler(FRHISamplerCreateInfo&& I_Desc)
    {
        const auto Type = I_Desc.Type;
        const auto Addr = I_Desc.AddressMode;
        auto ID = Registry->Register(std::move(I_Desc));
        LOG_DEBUG("CreateSampler: {} {} -> {}", Type, Addr, ID.GetHandle());
        return ID;
    }

    FRHIDescriptorSetID FRHI::
    CreateDescriptorSet(FRHIDescriptorSetCreateInfo&& I_Desc)
    {
        const auto BindingCount = I_Desc.Bindings.GetSize();
        auto ID = Registry->Register(std::move(I_Desc));
        LOG_DEBUG("CreateDescriptorSet: {} bindings -> {}",
                  BindingCount, ID.GetHandle());
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
