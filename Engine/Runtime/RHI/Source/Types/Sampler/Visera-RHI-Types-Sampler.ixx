module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Types.Sampler;
#define VISERA_MODULE_NAME "RHI.Types"
import Visera.RHI.Vulkan.Sampler;

export namespace Visera
{
    using FRHISampler       = FVulkanSampler;
}