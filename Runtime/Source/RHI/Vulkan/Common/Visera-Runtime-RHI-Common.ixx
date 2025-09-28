module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.Common;
#define VISERA_MODULE_NAME "Runtime.RHI"

export namespace Visera::RHI
{
    using EVulkanImageLayout   = vk::ImageLayout;
    using EVulkanPipelineStage = vk::PipelineStageFlagBits2;
    using EVulkanAccess        = vk::AccessFlagBits2;
    using EVulkanQueue         = vk::QueueFlagBits;
    using EVulkanLoadOp        = vk::AttachmentLoadOp;
    using EVulkanStoreOp       = vk::AttachmentStoreOp;
    using EVulkanImageType     = vk::ImageType;
    using EVulkanFormat        = vk::Format;
    using EVulkanImageUsage    = vk::ImageUsageFlagBits;
    using EVulkanImageAspect   = vk::ImageAspectFlagBits;
    using EVulkanSharingMode   = vk::SharingMode;
    using EVulkanSamplingRate  = vk::SampleCountFlagBits;
    using EVulkanImageTiling   = vk::ImageTiling;

    using FVulkanExtent2D      = vk::Extent2D;
    using FVulkanExtent3D      = vk::Extent3D;
    using FVulkanViewport      = vk::Viewport;
    using FVulkanRect2D        = vk::Rect2D;
    using FVulkanClearColor    = vk::ClearColorValue;
}

using namespace Visera;

template <>
struct fmt::formatter<RHI::EVulkanImageLayout>
{
    // Parse format specifiers (if any)
    constexpr auto parse(format_parse_context& I_Context) -> decltype(I_Context.begin())
    {
        return I_Context.begin();  // No custom formatting yet
    }

    // Corrected format function with const-correctness
    template <typename FormatContext>
    auto format(RHI::EVulkanImageLayout I_ImageLayout, FormatContext& I_Context) const
    -> decltype(I_Context.out())
    {
        const char* Name = "(WIP)";
        switch (I_ImageLayout)
        {
        case RHI::EVulkanImageLayout::eUndefined:                         Name = "Undefined"; break;
        case RHI::EVulkanImageLayout::eColorAttachmentOptimal:            Name = "Color"; break;
        case RHI::EVulkanImageLayout::eDepthAttachmentOptimal:            Name = "Depth"; break;
        case RHI::EVulkanImageLayout::eDepthStencilAttachmentOptimal:     Name = "DepthStencil"; break;
        case RHI::EVulkanImageLayout::eTransferSrcOptimal:                Name = "TransferSource"; break;
        case RHI::EVulkanImageLayout::eTransferDstOptimal:                Name = "TransferDestination"; break;
        case RHI::EVulkanImageLayout::ePresentSrcKHR:                     Name = "Present"; break;
        default: break;
        }
        return fmt::format_to(I_Context.out(),"{}", Name);
    }
};