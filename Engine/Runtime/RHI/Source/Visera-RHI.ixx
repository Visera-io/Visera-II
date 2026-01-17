module;
#include <Visera-RHI.hpp>
export module Visera.RHI;
#define VISERA_MODULE_NAME "RHI"
export import Visera.RHI.Common;
export import Visera.RHI.Types;
export import Visera.RHI.Resource;
export import Visera.RHI.CommandList;
       import Visera.RHI.Vulkan;
       import Visera.RHI.Registry;
       import Visera.Core.Types.Array;
       import Visera.Core.Delegate;
       import Visera.Global.Service;
       import Visera.Global.Log;
       import vulkan_hpp;

export namespace Visera
{
    using FRHIDrawCalls     = FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>;
    using FRHITransferCalls = FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>;

    class VISERA_RHI_API FRHI : public IGlobalService
    {
    public:
        TMulticastDelegate<>
        OnBeginFrame;
        TMulticastDelegate<>
        OnEndFrame;
        TUnicastDelegate<void(FRHIDrawCalls*, FRHIImageView*)>
        DebugUIDrawCalls;

        [[nodiscard]] FRHITextureHandle
        CreateTexture(FRHITextureCreateDesc&& I_TextureDesc);
        void
        DestroyTexture(FRHITextureHandle I_TextureHandle, Bool I_bTransient = False);
        [[nodiscard]] FRHIBufferHandle
        CreateBuffer(FRHIBufferCreateDesc&& I_BufferDesc);
        void
        DestroyBuffer(FRHIBufferHandle I_BufferHandle, Bool I_bTransient = False);

        [[nodiscard]] Bool
        BeginFrame();
        void
        EndFrame();
        void
        Submit(const FRHICommandList& I_CommandList);
        void
        Present();

        // Low-level API
        [[nodiscard]] inline const TUniquePtr<FVulkanDriver>&
        GetDriver(DEBUG_ONLY_FIELD(const std::source_location& I_Location = std::source_location::current()))  const
        {
            DEBUG_ONLY_FIELD(LOG_WARN("\"{}\" line:{} \"{}\" accessed the RHI driver.",
                             I_Location.file_name(),
                             I_Location.line(),
                             I_Location.function_name()));
            return Driver;
        };

    private:
        TUniquePtr<FVulkanDriver>   Driver;
        TUniquePtr<FRHIRegistry>    Registry;

        FVulkanCommandPool<EVulkanQueueFamily::Graphics>
        GraphicsCommandPool;
        FVulkanCommandPool<EVulkanQueueFamily::Transfer>
        TransferCommandPool;

        struct FFrame
        {
            FVulkanFence      Fence;
#if !defined(VISERA_OFFSCREEN_MODE)
            FVulkanSemaphore  SwapChainReadySemaphore;
#endif
            FVulkanSemaphore  RenderFinishedSemaphore;
            FRHIDrawCalls     DrawCalls;

            FVulkanSemaphore  TransferFinishedSemaphore;
            FRHITransferCalls TransferCalls;
        };
        TArray<FFrame> InFlightFrames;
        UInt8          FrameIndex = 0;

    public:
        FRHI() : IGlobalService(EName::RHI)
        {
            Dependencies =
            {
                EName::Platform,
#if !defined(VISERA_OFFSCREEN_MODE)
                EName::Window,
#endif
            };

            if (!OnBootstrap.TryBind([this]
            {
                Driver   = MakeUnique<FVulkanDriver>();
                Registry = MakeUnique<FRHIRegistry>(Driver);

                GraphicsCommandPool = Driver->CreateCommandPool<EVulkanQueueFamily::Graphics>
                (
                    False
                );
                TransferCommandPool = Driver->CreateCommandPool<EVulkanQueueFamily::Transfer>
                (
                    False
                );

#if !defined(VISERA_OFFSCREEN_MODE)
                auto Cmd = GraphicsCommandPool.CreateCommandBuffer(True);
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
                auto Fence = Driver->CreateFence(False);

                Driver->Submit(&Cmd, nullptr, nullptr, &Fence);
                if (!Fence.Wait())
                {
                    LOG_FATAL("Failed to init RHI SwapChain!");
                }

                InFlightFrames.Resize(Driver->GetSwapChain().Images.GetSize());
                for (auto& Frame : InFlightFrames)
                {
                    Frame.Fence = Driver->CreateFence(True);

                    Frame.SwapChainReadySemaphore = Driver->CreateSemaphore();

                    Frame.RenderFinishedSemaphore = Driver->CreateSemaphore();
                    Frame.DrawCalls = GraphicsCommandPool.CreateCommandBuffer(True);

                    Frame.TransferFinishedSemaphore = Driver->CreateSemaphore();
                    Frame.TransferCalls = TransferCommandPool.CreateCommandBuffer(True);
                }
#else
                InFlightFrames.Resize(1);
                for (auto& Frame : InFlightFrames)
                {
                    Frame.Fence = Driver->CreateFence(True);

                    Frame.RenderFinishedSemaphore = Driver->CreateSemaphore();
                    Frame.DrawCalls = GraphicsCommandPool.CreateCommandBuffer(True);

                    Frame.TransferFinishedSemaphore = Driver->CreateSemaphore();
                    Frame.TransferCalls = TransferCommandPool.CreateCommandBuffer(True);
                }
#endif
                return True;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                Driver->WaitIdle();
                InFlightFrames.Clear();
                GraphicsCommandPool = {};
                TransferCommandPool = {};
                Registry.reset();
                Driver.reset();
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        }
    };

    Bool FRHI::
    BeginFrame()
    {
        FrameIndex = (FrameIndex + 1) % InFlightFrames.GetSize();
        auto& CurrentFrame = InFlightFrames[FrameIndex];

        if (!CurrentFrame.Fence.Wait()) { return False; }

        Registry->CollectGarbage(FrameIndex);
#if !defined(VISERA_OFFSCREEN_MODE)
        if (Driver->WaitNextFrame(&CurrentFrame.SwapChainReadySemaphore))
        {
            if (!CurrentFrame.Fence.Reset())
            {
                LOG_ERROR("Failed to reset the Fence!");
                return False;
            }
        }
        else
        {
            LOG_ERROR("Failed to begin new frame!");
            return False;
        }
#endif
        CurrentFrame.TransferCalls.Reset();
        CurrentFrame.TransferCalls.Begin();
        CurrentFrame.DrawCalls.Reset();
        CurrentFrame.DrawCalls.Begin();
        OnBeginFrame.Broadcast();

        return True;
    }

    void FRHI::
    EndFrame()
    {
        auto& CurrentFrame = InFlightFrames[FrameIndex];
        OnEndFrame.Broadcast();

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

        Driver->Submit(&CurrentFrame.DrawCalls,
        &CurrentFrame.TransferFinishedSemaphore,
        &CurrentFrame.RenderFinishedSemaphore,
        &CurrentFrame.Fence);

        Registry->ClearGarbage();
    }

    void FRHI::
    Submit(const FRHICommandList& I_CommandList)
    {
        auto& Frame = InFlightFrames[FrameIndex];
        VISERA_ASSERT(Frame.DrawCalls.IsRecording());

        for (auto Command : I_CommandList)
        {
            if (Command.PayloadPtrAligned == nullptr) { continue; }

            switch (Command.Type)
            {
            case ECommandType::ConvertImageLayout:
                {
                    const auto* Payload = reinterpret_cast<const FRHICommandList::FConvertImageLayout*>(Command.PayloadPtrAligned);

                    auto* Texture = Registry->GetTexture(Payload->Image);
                    VISERA_ASSERT(Texture);

                    Frame.DrawCalls.ConvertImageLayout(
                        Texture->GetImage(),
                        TypeCast(Payload->NewLayout),
                        EVulkanGraphicsStage::TopOfPipe,
                        EVulkanGraphicsAccess::None,
                        EVulkanGraphicsStage::BottomOfPipe,
                        EVulkanGraphicsAccess::None
                    );
                    break;
                }
            case ECommandType::ClearColorImage:
                {
                    const auto* Payload = reinterpret_cast<const FRHICommandList::FClearColorImage*>(Command.PayloadPtrAligned);

                    auto* Texture = Registry->GetTexture(Payload->Image);
                    VISERA_ASSERT(Texture);

                    Frame.DrawCalls.ClearColorImage(Texture->GetImage(),{
                            Payload->ClearColor.R,
                            Payload->ClearColor.G,
                            Payload->ClearColor.B,
                            Payload->ClearColor.A,
                    });
                    break;
                }
            case ECommandType::BlitImage:
                {
                    const auto* Payload = reinterpret_cast<const FRHICommandList::FBlitImage*>(Command.PayloadPtrAligned);

                    auto* SrcTexture = Registry->GetTexture(Payload->SrcImage);
                    auto* DstTexture = Registry->GetTexture(Payload->DstImage);
                    VISERA_ASSERT(SrcTexture && DstTexture);

                    Frame.DrawCalls.BlitImage(SrcTexture->GetImage(), DstTexture->GetImage());
                    break;
                }
            case ECommandType::BlitToSwapChain:
                {
                    const auto* Payload = reinterpret_cast<const FRHICommandList::FBlitToSwapChain*>(Command.PayloadPtrAligned);

                    auto* Texture = Registry->GetTexture(Payload->Image);
                    VISERA_ASSERT(Texture);
                    auto  SwapChainImage = Driver->GetSwapChain().GetCurrentImage();
                    Frame.DrawCalls.ConvertImageLayout(SwapChainImage,
                        TypeCast(ERHIImageLayout::TransferDst),
                        EVulkanGraphicsStage::TopOfPipe,
                        EVulkanGraphicsAccess::None,
                        EVulkanGraphicsStage::BottomOfPipe,
                        EVulkanGraphicsAccess::None);
                    Frame.DrawCalls.BlitImage(Texture->GetImage(), SwapChainImage);
                    Frame.DrawCalls.ConvertImageLayout(SwapChainImage,
                        TypeCast(ERHIImageLayout::Present),
                        EVulkanGraphicsStage::TopOfPipe,
                        EVulkanGraphicsAccess::None,
                        EVulkanGraphicsStage::BottomOfPipe,
                        EVulkanGraphicsAccess::None);
                    break;
                }
            default: LOG_ERROR("Unknown Command!"); break;
            }
        }
    }

    void FRHI::
    Present()
    {
        auto& CurrentFrame = InFlightFrames[FrameIndex];
        if (!Driver->Present(&CurrentFrame.RenderFinishedSemaphore))
        {
            LOG_ERROR("Failed to present!");
        }
    }

    FRHITextureHandle FRHI::
    CreateTexture(FRHITextureCreateDesc&& I_TextureDesc)
    {
        auto Handle  = Registry->Register(std::move(I_TextureDesc));
        auto Texture = Registry->GetTexture(Handle);
        auto& CurrentFrame = InFlightFrames[FrameIndex];
        CurrentFrame.DrawCalls.ConvertImageLayout(
            Texture->GetImage(),
            vk::ImageLayout::eColorAttachmentOptimal,
            EVulkanGraphicsStage::TopOfPipe,
            EVulkanGraphicsAccess::None,
            EVulkanGraphicsStage::BottomOfPipe,
            EVulkanGraphicsAccess::None);
        return Handle;
    }

    void FRHI::
    DestroyTexture(FRHITextureHandle I_TextureHandle, Bool I_bTransient)
    {
        UInt8 RetiredFrame = (FrameIndex + I_bTransient) % InFlightFrames.GetSize();
        Registry->Unregister(I_TextureHandle, RetiredFrame);
    }

    FRHIBufferHandle FRHI::
    CreateBuffer(FRHIBufferCreateDesc&& I_BufferDesc)
    {
        auto Handle = Registry->Register(std::move(I_BufferDesc));
        return Handle;
    }

    void FRHI::
    DestroyBuffer(FRHIBufferHandle I_BufferHandle, Bool I_bTransient)
    {
        UInt8 RetiredFrame = (FrameIndex + I_bTransient) % InFlightFrames.GetSize();
        Registry->Unregister(I_BufferHandle, RetiredFrame);
    }
}
