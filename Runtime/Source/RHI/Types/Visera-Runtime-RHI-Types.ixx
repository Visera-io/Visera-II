module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan.hpp>
export module Visera.Runtime.RHI.Types;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan;
import Visera.Core.Log;
import Visera.Core.Traits.Flags;

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

    enum class ERHISetBindingType
    {
        Undefined            = 0,
        CombinedImageSampler = 1 << 0,
        UniformBuffer        = 1 << 1,
    };
    VISERA_MAKE_FLAGS(ERHISetBindingType);

    enum class ERHIShaderStages : UInt32
    {
        Undefined = 0,
        Vertex    = 1 << 0,
        Fragment  = 1 << 1,
    };
    VISERA_MAKE_FLAGS(ERHIShaderStages);

    struct VISERA_RUNTIME_API FRHISetBinding
    {
        UInt32              Index        {0};
        ERHISetBindingType  Type         {ERHISetBindingType::Undefined};
        UInt32              Count        {0};
        ERHIShaderStages    ShaderStages {ERHIShaderStages::Undefined};

        UInt64              ImmutableSamplerID {0};
    };

    struct VISERA_RUNTIME_API FRHISetLayout
    {

    };
}