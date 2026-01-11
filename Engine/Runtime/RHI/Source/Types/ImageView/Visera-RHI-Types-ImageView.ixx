module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Types.ImageView;
#define VISERA_MODULE_NAME "RHI.Types"
import Visera.RHI.Vulkan.Image;

export namespace Visera
{
    using FRHIImageView = FVulkanImageView;
}