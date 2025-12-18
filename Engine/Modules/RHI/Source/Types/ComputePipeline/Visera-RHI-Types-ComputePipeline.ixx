module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Types.ComputePipeline;
#define VISERA_MODULE_NAME "RHI.Types"
import Visera.RHI.Vulkan.Pipeline.Compute;

export namespace Visera
{
    using FRHIComputePipeline = FVulkanComputePipeline;
}