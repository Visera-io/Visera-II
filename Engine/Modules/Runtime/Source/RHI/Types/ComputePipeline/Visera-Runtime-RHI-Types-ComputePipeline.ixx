module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Types.ComputePipeline;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Pipeline.Compute;

export namespace Visera
{
    using FRHIComputePipeline = FVulkanComputePipeline;
}