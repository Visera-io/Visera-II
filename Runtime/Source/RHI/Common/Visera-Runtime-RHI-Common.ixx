module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Common;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan;
import Visera.Core.Log;

export namespace Visera
{
    namespace RHI
    {
        using EQueue            = EVulkanQueue;
        using EImageType        = EVulkanImageType;
        using EFormat           = EVulkanFormat;
        using EImageUsage       = EVulkanImageUsage;
        using EImageLayout      = EVulkanImageLayout;
        using EPipelineStage    = EVulkanPipelineStage;
        using EAccess           = EVulkanAccess;
        using EShaderStage      = EVulkanShaderStage;
        using EMemoryPoolFlag   = EVulkanMemoryPoolFlag;
        using EBufferUsage      = EVulkanBufferUsage;
        using EDescriptorType   = EVulkanDescriptorType;

        using FBuffer           = FVulkanBuffer;
        using FImage            = FVulkanImage;
        using FImageView        = FVulkanImageView;
        using FPipelineLayout   = FVulkanPipelineLayout;
        using FRenderPipeline   = FVulkanRenderPipeline;
        using FCommandBuffer    = FVulkanCommandBuffer;
        using FDescriptorSet    = FVulkanDescriptorSet;
        using FSampler          = FVulkanSampler;
        using FDescriptorSetLayoutBinding = FVulkanDescriptorSetLayoutBinding;
    }
}