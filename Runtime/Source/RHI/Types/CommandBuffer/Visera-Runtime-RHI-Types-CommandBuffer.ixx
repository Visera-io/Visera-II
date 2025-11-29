module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Types.CommandBuffer;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.CommandBuffer;

export namespace Visera
{
    using FRHICommandBuffer = FVulkanCommandBuffer;
}