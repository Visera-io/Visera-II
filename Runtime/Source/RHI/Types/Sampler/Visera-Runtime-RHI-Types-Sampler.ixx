module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Types.Sampler;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Sampler;

export namespace Visera
{
    using FRHISampler       = FVulkanSampler;
}