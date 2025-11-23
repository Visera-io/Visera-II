module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan.hpp>
export module Visera.Runtime.RHI.Common;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan;
import Visera.Core.Log;

export namespace Visera
{
    using EQueue            = vk::QueueFlagBits;
    using EImageType        = vk::ImageType;
    using EFormat           = vk::Format;
    using ELoadOp           = vk::AttachmentLoadOp;
    using EStoreOp          = vk::AttachmentStoreOp;
    using EImageUsage       = vk::ImageUsageFlagBits;
    using EImageLayout      = vk::ImageLayout;
    using EImageViewType    = vk::ImageViewType;
    using EImageAspect      = vk::ImageAspectFlagBits;
    using EPipelineStage    = vk::PipelineStageFlagBits2;
    using EAccess           = vk::AccessFlagBits2;
    using EShaderStage      = vk::ShaderStageFlagBits;
    using EFilter           = vk::Filter;
    using ESamplerAddressMode= vk::SamplerAddressMode;
    using EMemoryPoolFlag   = EVulkanMemoryPoolFlag;
    using EBufferUsage      = vk::BufferUsageFlagBits;
    using EDescriptorType   = vk::DescriptorType;

    using FBuffer           = FVulkanBuffer;
    using FImage            = FVulkanImage;
    using FImageView        = FVulkanImageView;
    using FPipelineLayout   = FVulkanPipelineLayout;
    using FRenderPipeline   = FVulkanRenderPipeline;
    using FCommandBuffer    = FVulkanCommandBuffer;
    using FDescriptorSet    = FVulkanDescriptorSet;
    using FSampler          = FVulkanSampler;
    using FViewport         = vk::Viewport;
    using FPushConstant     = vk::PushConstantRange;
    using FDescriptorSetLayout  = FVulkanDescriptorSetLayout;
    using FDescriptorSetBinding = vk::DescriptorSetLayoutBinding;
}