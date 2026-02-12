module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.SwapChain;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Common;
       import Visera.Runtime.RHI.Vulkan;
       import Visera.Runtime.Window;
       import Visera.Core.Containers.Array;
       import Visera.Core.Delegate;
       import Visera.Core.Log;
       import Visera.Core.Types.Optional;
       import Visera.Core.Types.String;
       import vulkan_hpp;

export namespace Visera
{
    using FRHIDrawCalls     = FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>;
    using FRHITransferCalls = FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>;

    /// Per-frame resources for a swap chain.
    struct VISERA_RUNTIME_API FRHIInFlightFrame
    {
        FVulkanFence      ExecuteFence;
        FVulkanSemaphore  SwapChainReadySemaphore;
        FVulkanSemaphore  RenderFinishedSemaphore;    // Offscreen only
        FRHIDrawCalls     DrawCalls;
        FVulkanSemaphore  TransferFinishedSemaphore;
        FRHITransferCalls TransferCalls;
    };

    /// RHI-level swap chain context: frame resources and synchronization state.
    struct VISERA_RUNTIME_API FRHISwapChain
    {
        FWindow*                     Window { nullptr };  // For Driver lookup; nullptr = offscreen
        TArray<FRHIInFlightFrame>    InFlightFrames;
        TArray<FVulkanSemaphore>     RenderFinishedSemaphores;
        UInt8                        FrameIndex = 0;
        UInt8                        LastSubmittedImageIndex = 0;

        /// Initialize context. I_Window=nullptr: offscreen (1 frame). I_Window non-null: from window swap chain.
        void Initialize(
            FVulkanDriver*                                      I_Driver,
            FVulkanGraphicsCommandPool*   I_GraphicsPool,
            FVulkanTransferCommandPool*   I_TransferPool,
            FWindow*                                            I_Window);

        /// Subscribe to window resize; reinitializes this swap chain on resize.
        void SubscribeToResize(
            FVulkanDriver*                                      I_Driver,
            FVulkanGraphicsCommandPool*   I_GraphicsPool,
            FVulkanTransferCommandPool*   I_TransferPool,
            FWindow*                                            I_Window,
            FStringView                                         I_RuntimeName = "RHI");
        void UnsubscribeFromResize();

    private:
        using FResizeHandle = TMulticastDelegate<FWindow*>::FHandle;
        TOptional<FResizeHandle> ResizeHandle;
    };

    void FRHISwapChain::
    Initialize(
        FVulkanDriver*                                       I_Driver,
        FVulkanGraphicsCommandPool*    I_GraphicsPool,
        FVulkanTransferCommandPool*    I_TransferPool,
        FWindow*                                             I_Window)
    {
        Window = I_Window;
        if (!I_Window)
        {
            InFlightFrames.Resize(1);
            RenderFinishedSemaphores.Clear();
            for (auto& Frame : InFlightFrames)
            {
                Frame.ExecuteFence = I_Driver->CreateFence(True);
                Frame.RenderFinishedSemaphore = I_Driver->CreateSemaphore();
                Frame.DrawCalls = I_GraphicsPool->CreateCommandBuffer(True);
                Frame.TransferFinishedSemaphore = I_Driver->CreateSemaphore();
                Frame.TransferCalls = I_TransferPool->CreateCommandBuffer(True);
            }
            return;
        }
        auto* SC = I_Driver->GetSwapChain(I_Window);
        if (!SC) { return; }
        InFlightFrames.Resize(SC->Images.GetSize());
        RenderFinishedSemaphores.Clear();
        for (UInt32 Idx = 0; Idx < SC->Images.GetSize(); ++Idx)
        {
            RenderFinishedSemaphores.EmplaceBack(I_Driver->CreateSemaphore());
        }
        for (auto& Frame : InFlightFrames)
        {
            Frame.ExecuteFence = I_Driver->CreateFence(True);
            Frame.SwapChainReadySemaphore = I_Driver->CreateSemaphore();
            Frame.DrawCalls = I_GraphicsPool->CreateCommandBuffer(True);
            Frame.TransferFinishedSemaphore = I_Driver->CreateSemaphore();
            Frame.TransferCalls = I_TransferPool->CreateCommandBuffer(True);
        }
        FRHIDrawCalls Cmd = I_GraphicsPool->CreateCommandBuffer(True);
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
        FVulkanFence Fence = I_Driver->CreateFence(False);
        I_Driver->Submit(&Cmd, nullptr, nullptr, &Fence);
        if (!Fence.Wait())
        { LOG_FATAL("Failed to init RHI SwapChain!"); }
    }

    void FRHISwapChain::
    SubscribeToResize(
        FVulkanDriver*                                       I_Driver,
        FVulkanGraphicsCommandPool*    I_GraphicsPool,
        FVulkanTransferCommandPool*    I_TransferPool,
        FWindow*                                             I_Window,
        FStringView                                          I_RuntimeName)
    {
        if (!I_Window || ResizeHandle.HasValue()) { return; }
        auto* Ctx = this;
        ResizeHandle = I_Window->OnResized.Subscribe([Ctx, I_Driver, I_GraphicsPool, I_TransferPool, I_RuntimeName](FWindow* I_Win)
        {
            if (I_Win->GetWidth() == 0 || I_Win->GetHeight() == 0)
            {
                LOG_TRACE("({}) Skip SwapChain recreation while minimized ({}x{}).", I_RuntimeName, I_Win->GetWidth(), I_Win->GetHeight());
                return;
            }
            LOG_DEBUG("({}) Recreating SwapChain ({}x{}) for window (title:{}).", I_RuntimeName, I_Win->GetWidth(), I_Win->GetHeight(), I_Win->GetTitle());
            I_Win->OnResized.Unsubscribe(Ctx->ResizeHandle.GetValue());
            Ctx->ResizeHandle = NullOpt;
            I_Driver->WaitIdle();
            *Ctx = FRHISwapChain{};
            I_Driver->RecreateSwapChain(I_Win, I_Win->GetWidth(), I_Win->GetHeight());
            Ctx->Initialize(I_Driver, I_GraphicsPool, I_TransferPool, I_Win);
            Ctx->SubscribeToResize(I_Driver, I_GraphicsPool, I_TransferPool, I_Win, I_RuntimeName);
        });
    }

    void FRHISwapChain::
    UnsubscribeFromResize()
    {
        if (Window && ResizeHandle.HasValue())
        {
            Window->OnResized.Unsubscribe(ResizeHandle.GetValue());
            ResizeHandle = NullOpt;
        }
    }
}
