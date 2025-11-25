module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Types.Image;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Image;

export namespace Visera
{
    using FRHIImage         = FVulkanImageView;
}