module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.Common;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Interface.Common;
import Visera.Core.Log;

export namespace Visera::RHI
{
    vk::ImageLayout
    AutoCast(EImageLayout I_Layout)
    {
        switch (I_Layout)
        {
        case EImageLayout::Color:
            return vk::ImageLayout::eColorAttachmentOptimal;
        case EImageLayout::Depth:
            return vk::ImageLayout::eDepthAttachmentOptimal;
        case EImageLayout::Present:
            return vk::ImageLayout::ePresentSrcKHR;
        case EImageLayout::TransferSource:
            return vk::ImageLayout::eTransferSrcOptimal;
        case EImageLayout::TransferDestination:
            return vk::ImageLayout::eTransferDstOptimal;
        default:
            LOG_FATAL("Failed to cast the image layout"
                      " -- Unsupported image layout!");
        }
        return vk::ImageLayout::eUndefined;
    }

    vk::PipelineStageFlagBits2
    AutoCast(EPipelineStage I_PipelineStage)
    {
        switch (I_PipelineStage)
        {
        case EPipelineStage::Top:
            return vk::PipelineStageFlagBits2::eTopOfPipe;
        case EPipelineStage::ColorOutput:
            return vk::PipelineStageFlagBits2::eColorAttachmentOutput;
        default:
            LOG_FATAL("Failed to cast the pipeline stage"
                      " -- Unsupported pipeline stage!");
        }
        return vk::PipelineStageFlagBits2::eNone;
    }

    vk::AccessFlagBits2
    AutoCast(EAccess I_Access)
    {
        switch (I_Access)
        {
        case EAccess::W_Color:
            return vk::AccessFlagBits2::eColorAttachmentWrite;
        default:
            LOG_FATAL("Failed to cast the access"
                      " -- Unsupported access!");
        }
        return vk::AccessFlagBits2::eNone;
    }
}