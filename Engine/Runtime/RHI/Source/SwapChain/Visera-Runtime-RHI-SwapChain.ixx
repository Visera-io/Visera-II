module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.SwapChain;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Common;
       import Visera.Core.OS.Thread.Sync.Atomic;
       import Visera.Core.OS.Time;
       import Visera.Core.OS.Thread.Sync.Event;
       import Visera.Runtime.RHI.Registry;
       import Visera.Runtime.RHI.Vulkan;
       import Visera.Runtime.Window;
       import Visera.Core.Containers.Array;
       import Visera.Core.Types.Pointer;
       import Visera.Core.Log;
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
        FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>
        TransferCalls;
        FVulkanSemaphore  TransferFinishedSemaphore;
    };

    /// RHI-level swap chain context: frame resources and synchronization state.
    struct VISERA_RUNTIME_API FRHISwapChain
    {
        FWindow*                  Window {nullptr};  // For Driver lookup; nullptr = offscreen
        TArray<FRHIInFlightFrame> InFlightFrames;
        TArray<FVulkanSemaphore>  RenderFinishedSemaphores;
        UInt8                     FrameIndex = 0;
        FRHITextureID             CachedProxyTextureID;
        Bool                      bFrameActive = False;
        TAtomic<Bool>             bDirty {False};   // Set when recreate requested; cleared when ExecuteRecreateSwapChain finishes. Read from main thread.
        TAtomic<Bool>             bDestroyed {False};  // Set on main thread when DestroySwapChain called; read from graphics thread.
        TAtomic<Bool>             bMinimized {False};  // Updated from main thread window state; read from graphics thread.
        UInt32                    CachedWidth {0}; // Window size when swapchain was created/recreated; used to detect resize in BeginFrame.
        UInt32                    CachedHeight {0};
        UInt32                    PendingRecreateWidth {0};  // Last submitted size for recreate; task runs with this so we dedupe by "last size".
        UInt32                    PendingRecreateHeight {0};
        Bool                      bRecreateEnqueued {False}; // Only one RecreateSwapChain task in flight per swapchain.
        /** Frames enqueued for Present but not yet completed on RHI thread. Used for BeginFrame backpressure. */
        TAtomic<UInt32>           PendingPresentCount {0};
        /** Signalled by RHI thread when PendingPresentCount drops below MaxInFlight; Graphics thread waits on this for linear submit. */
        TUniquePtr<FEvent>        FrameSlotFreeEvent;
        /** FPS for windowed swap chains (EMA-smoothed); updated by RHI thread after each Present. */
        TAtomic<Float>            FrameRate {0.f};
        /** Timer for frame interval (RHI thread): Elapsed() then Reset() after each Present. */
        FHiResClock               FrameTimer;

        FRHISwapChain() = default;
        FRHISwapChain(FRHISwapChain&& I_Other) noexcept;
        FRHISwapChain& operator=(FRHISwapChain&& I_Other) noexcept;
        FRHISwapChain(const FRHISwapChain&) = delete;
        FRHISwapChain& operator=(const FRHISwapChain&) = delete;

        /// Initialize context. I_Window=nullptr: headless (own InFlightFrames, no Vulkan surface). I_Window non-null: windowed swap chain. In-flight count uses kMaxInFlightFrames.
        void Initialize(
            FVulkanDriver*                I_Driver,
            FVulkanGraphicsCommandPool*   I_GraphicsPool,
            FVulkanTransferCommandPool*   I_TransferPool,
            FWindow*                      I_Window);

        /// Rebuild frame resources in-place after swap chain recreation. Preserves PendingPresentCount and FrameSlotFreeEvent to avoid underflow when Present tasks are still in flight.
        void Reinitialize(
            FVulkanDriver*                I_Driver,
            FVulkanGraphicsCommandPool*   I_GraphicsPool,
            FVulkanTransferCommandPool*   I_TransferPool);

    private:
        void MoveFrom(FRHISwapChain&& I_Other) noexcept;
        void BuildFrameResources(
            FVulkanDriver*                I_Driver,
            FVulkanGraphicsCommandPool*   I_GraphicsPool,
            FVulkanTransferCommandPool*   I_TransferPool);
        void BuildHeadlessFrameResources(
            FVulkanDriver*                I_Driver,
            FVulkanGraphicsCommandPool*   I_GraphicsPool,
            FVulkanTransferCommandPool*   I_TransferPool);
    };

    void FRHISwapChain::MoveFrom(FRHISwapChain&& I_Other) noexcept
    {
        Window                   = I_Other.Window;
        InFlightFrames           = std::move(I_Other.InFlightFrames);
        RenderFinishedSemaphores = std::move(I_Other.RenderFinishedSemaphores);
        FrameIndex               = I_Other.FrameIndex;
        CachedProxyTextureID     = I_Other.CachedProxyTextureID;
        bFrameActive             = I_Other.bFrameActive;
        CachedWidth              = I_Other.CachedWidth;
        CachedHeight             = I_Other.CachedHeight;
        PendingRecreateWidth     = I_Other.PendingRecreateWidth;
        PendingRecreateHeight    = I_Other.PendingRecreateHeight;
        bRecreateEnqueued        = I_Other.bRecreateEnqueued;
        FrameSlotFreeEvent       = std::move(I_Other.FrameSlotFreeEvent);
        FrameTimer               = std::move(I_Other.FrameTimer);
        FrameRate.Store(I_Other.FrameRate.Load(EMemoryOrder::Relaxed), EMemoryOrder::Relaxed);
        bDirty.Store(I_Other.bDirty.Load(EMemoryOrder::Relaxed), EMemoryOrder::Relaxed);
        bDestroyed.Store(I_Other.bDestroyed.Load(EMemoryOrder::Relaxed), EMemoryOrder::Relaxed);
        bMinimized.Store(I_Other.bMinimized.Load(EMemoryOrder::Relaxed), EMemoryOrder::Relaxed);
        PendingPresentCount.Store(I_Other.PendingPresentCount.Load(EMemoryOrder::Relaxed), EMemoryOrder::Relaxed);

        I_Other.PendingPresentCount.Store(0, EMemoryOrder::Relaxed);
        I_Other.Window              = nullptr;
        I_Other.FrameIndex          = 0;
        I_Other.bFrameActive        = False;
        I_Other.CachedWidth         = 0;
        I_Other.CachedHeight        = 0;
        I_Other.PendingRecreateWidth  = 0;
        I_Other.PendingRecreateHeight = 0;
        I_Other.bRecreateEnqueued   = False;
    }

    FRHISwapChain::FRHISwapChain(FRHISwapChain&& I_Other) noexcept
    { MoveFrom(std::move(I_Other)); }

    FRHISwapChain& FRHISwapChain::operator=(FRHISwapChain&& I_Other) noexcept
    {
        if (this != &I_Other) { MoveFrom(std::move(I_Other)); }
        return *this;
    }

    void FRHISwapChain::
    Initialize(
        FVulkanDriver*                 I_Driver,
        FVulkanGraphicsCommandPool*    I_GraphicsPool,
        FVulkanTransferCommandPool*    I_TransferPool,
        FWindow*                       I_Window)
    {
        Window = I_Window;
        if (I_Window)
        {
            BuildFrameResources(I_Driver, I_GraphicsPool, I_TransferPool);
            CachedWidth  = I_Window->GetWidth();
            CachedHeight = I_Window->GetHeight();
        }
        else
        {
            BuildHeadlessFrameResources(I_Driver, I_GraphicsPool, I_TransferPool);
        }
        FrameSlotFreeEvent = MakeUnique<FEvent>();
        FrameTimer.Reset();
    }

    void FRHISwapChain::
    Reinitialize(
        FVulkanDriver*              I_Driver,
        FVulkanGraphicsCommandPool* I_GraphicsPool,
        FVulkanTransferCommandPool* I_TransferPool)
    {
        InFlightFrames.Clear();
        RenderFinishedSemaphores.Clear();
        if (Window)
        {
            BuildFrameResources(I_Driver, I_GraphicsPool, I_TransferPool);
            CachedWidth  = Window->GetWidth();
            CachedHeight = Window->GetHeight();
        }
        else
        {
            BuildHeadlessFrameResources(I_Driver, I_GraphicsPool, I_TransferPool);
        }
        FrameIndex   = 0;
        bFrameActive = False;
        FrameTimer.Reset();
    }

    void FRHISwapChain::
    BuildFrameResources(
        FVulkanDriver*              I_Driver,
        FVulkanGraphicsCommandPool* I_GraphicsPool,
        FVulkanTransferCommandPool* I_TransferPool)
    {
        auto* SC = I_Driver->GetSwapChain(Window);
        if (!SC) { LOG_WARN("BuildFrameResources: no vulkan swapchain!"); return; }
        const UInt32 ImageCount    = static_cast<UInt32>(SC->Images.GetSize());
        const UInt32 InFlightCount = (ImageCount == 0) ? 1u
            : (kMaxInFlightFrames < ImageCount ? kMaxInFlightFrames : ImageCount);
        InFlightFrames.Resize(InFlightCount);
        RenderFinishedSemaphores.Clear();
        for (UInt32 Idx = 0; Idx < ImageCount; ++Idx)
        { RenderFinishedSemaphores.EmplaceBack(I_Driver->CreateSemaphore()); }
        for (auto& Frame : InFlightFrames)
        {
            Frame.ExecuteFence              = I_Driver->CreateFence(True);
            Frame.SwapChainReadySemaphore   = I_Driver->CreateSemaphore();
            Frame.GraphicsCalls             = I_GraphicsPool->CreateCommandBuffer(True);
            Frame.TransferFinishedSemaphore = I_Driver->CreateSemaphore();
            Frame.TransferCalls             = I_TransferPool->CreateCommandBuffer(True);
        }
    }

    void FRHISwapChain::
    BuildHeadlessFrameResources(
        FVulkanDriver*              I_Driver,
        FVulkanGraphicsCommandPool* I_GraphicsPool,
        FVulkanTransferCommandPool* I_TransferPool)
    {
        const UInt32 Count = (kMaxInFlightFrames > 0) ? kMaxInFlightFrames : 2u;
        InFlightFrames.Resize(Count);
        for (auto& Frame : InFlightFrames)
        {
            Frame.ExecuteFence              = I_Driver->CreateFence(True);
            Frame.SwapChainReadySemaphore   = I_Driver->CreateSemaphore();
            Frame.GraphicsCalls             = I_GraphicsPool->CreateCommandBuffer(True);
            Frame.TransferFinishedSemaphore = I_Driver->CreateSemaphore();
            Frame.TransferCalls             = I_TransferPool->CreateCommandBuffer(True);
        }
    }
}
