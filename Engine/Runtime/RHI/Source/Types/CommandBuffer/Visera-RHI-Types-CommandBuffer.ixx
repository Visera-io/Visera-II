module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Types.CommandBuffer;
#define VISERA_MODULE_NAME "RHI.Types"
import Visera.RHI.Vulkan.CommandBuffer;
import Visera.RHI.Vulkan.Common;

export namespace Visera
{
    using FRHIDrawCommandList = FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>;
}