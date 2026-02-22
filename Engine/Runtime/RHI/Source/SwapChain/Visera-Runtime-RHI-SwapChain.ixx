module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.SwapChain;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Common;
       import Visera.Runtime.RHI.Registry;
       import Visera.Runtime.RHI.Vulkan;
       import Visera.Runtime.Window;
       import Visera.Core.Containers.Array;
       import Visera.Core.Types.Pointer;
       import Visera.Core.Delegate;
       import Visera.Core.Log;
       import Visera.Core.Types.Function;
       import Visera.Core.Types.Optional;
       import Visera.Core.Types.String;
       import vulkan_hpp;

export namespace Visera
{
    /// Per-frame resources for a swap chain.
    struct VISERA_RUNTIME_API FRHIInFlightFrame
    {
        FVulkanFence      ExecuteFence;
        FVulkanSemaphore  SwapChainReadySemaphore;
        FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>
        GraphicsCalls;
        FVulkanSemaphore  RenderFinishedSemaphore;    // Offscreen only
        FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>
        TransferCalls;
        FVulkanSemaphore  TransferFinishedSemaphore;
    };

    /// RHI-level swap chain context: frame resources and synchronization state.
    struct VISERA_RUNTIME_API FRHISwapChain
    {
        TWeakPtr<FWindow>            Window;  // For Driver lookup; empty = offscreen
        TArray<FRHIInFlightFrame>    InFlightFrames;
        TArray<FVulkanSemaphore>     RenderFinishedSemaphores;
        UInt8                        FrameIndex = 0;
        UInt8                        LastSubmittedImageIndex = 0;
        Bool                         bFrameActive {False};
        FRHITextureID                CachedProxyTextureID;

        /// Initialize context. I_Window=nullptr: offscreen (1 frame). I_Window non-null: from window swap chain.
        void Initialize(
            FVulkanDriver*                                      I_Driver,
            FVulkanGraphicsCommandPool*   I_GraphicsPool,
            FVulkanTransferCommandPool*   I_TransferPool,
            TSharedPtr<FWindow>                                 I_Window);

        /// Subscribe to window resize; reinitializes this swap chain on resize.
        void SubscribeToResize(
            FVulkanDriver*                                      I_Driver,
            FVulkanGraphicsCommandPool*   I_GraphicsPool,
            FVulkanTransferCommandPool*   I_TransferPool,
            FWindow*                                            I_Window,
            const TFunction<void(FWindow*, UInt32, UInt32)>& I_OnResizeRequested);
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
        TSharedPtr<FWindow>                                  I_Window)
    {
        if (I_Window) { Window = I_Window; }
        if (!I_Window)
        {
            InFlightFrames.Resize(1);
            RenderFinishedSemaphores.Clear();
            for (auto& Frame : InFlightFrames)
            {
                Frame.ExecuteFence = I_Driver->CreateFence(True);
                Frame.RenderFinishedSemaphore = I_Driver->CreateSemaphore();
                Frame.GraphicsCalls = I_GraphicsPool->CreateCommandBuffer(True);
                Frame.TransferFinishedSemaphore = I_Driver->CreateSemaphore();
                Frame.TransferCalls = I_TransferPool->CreateCommandBuffer(True);
            }
            return;
        }
        auto* SC = I_Driver->GetSwapChain(I_Window.Get());
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
            Frame.GraphicsCalls = I_GraphicsPool->CreateCommandBuffer(True);
            Frame.TransferFinishedSemaphore = I_Driver->CreateSemaphore();
            Frame.TransferCalls = I_TransferPool->CreateCommandBuffer(True);
        }
        auto Cmd = I_GraphicsPool->CreateCommandBuffer(True);
        Cmd.Begin();
        for (auto& Image : SC->Images)
        {
            Cmd.ConvertImageLayout(&Image,
                vk::ImageLayout::eUndefined,
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
        FVulkanDriver*              I_Driver,
        FVulkanGraphicsCommandPool* I_GraphicsPool,
        FVulkanTransferCommandPool* I_TransferPool,
        FWindow*                    I_Window,
        const TFunction<void(FWindow*, UInt32, UInt32)>& I_OnResizeRequested)
    {
        if (!I_Window || ResizeHandle.HasValue()) { return; }
        ResizeHandle = I_Window->OnResized.Subscribe([I_OnResizeRequested](FWindow* I_Win)
        {
            if (!I_OnResizeRequested) { return; }
            if (I_Win->GetWidth() == 0 || I_Win->GetHeight() == 0)
            {
                LOG_TRACE("({}) Skip SwapChain recreation while minimized ({}x{}).", I_Win->GetTitle(), I_Win->GetWidth(), I_Win->GetHeight());
                return;
            }
            LOG_DEBUG("({}) Recreating SwapChain ({}x{}) for window (title:{}).", I_Win->GetTitle(), I_Win->GetWidth(), I_Win->GetHeight(), I_Win->GetTitle());
            I_OnResizeRequested(I_Win, I_Win->GetWidth(), I_Win->GetHeight());
        });
    }

    void FRHISwapChain::
    UnsubscribeFromResize()
    {
        if (auto Win = Window.Lock(); Win && ResizeHandle.HasValue())
        {
            Win->OnResized.Unsubscribe(ResizeHandle.GetValue());
            ResizeHandle = NullOpt;
        }
    }
}
