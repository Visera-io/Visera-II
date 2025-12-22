module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Frame;
#define VISERA_MODULE_NAME "RHI.Frame"
import Visera.Runtime.Log;
import Visera.RHI.Vulkan.Sync;
import Visera.RHI.Types;

export namespace Visera
{
    class VISERA_RHI_API FRHIFrame
    {
    public:

    private:
        TSharedPtr<FVulkanFence>         Fence;
        TSharedPtr<FRHICommandBuffer>    DrawCalls;
#if !defined(VISERA_OFFSCREEN_MODE)
        TSharedPtr<FVulkanSemaphore>     SwapChainImageAvailable;
#endif
    };
}