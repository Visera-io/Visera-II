module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan.hpp>
export module Visera.Runtime.RHI.Types;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan;
import Visera.Core.Log;
import Visera.Core.Hash.GoldenRatio;
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

    enum class ERHIFormat
    {
        R8G8B8_Float,
        R8G8B8_sRGB,
        R8G8B8_UNorm,

        B8G8R8_Float,
        B8G8R8_sRGB,
        B8G8R8_UNorm,

        R8G8B8A8_Float,
        R8G8B8A8_sRGB,
        R8G8B8A8_UNorm,

        B8G8R8A8_Float,
        B8G8R8A8_sRGB,
        B8G8R8A8_UNorm,

        Vector4F = R8G8B8A8_Float,
    };

    enum class ERHISetBindingType
    {
        Undefined = 0,
        CombinedImageSampler,
        UniformBuffer,
        StorageBuffer,
    };
    
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

        UInt64              ImmutableSamplerID {0 /*None = 0*/};

        // ---- Factories ----
        static constexpr [[nodiscard]] FRHISetBinding
        UniformBuffer(UInt32 I_Index, ERHIShaderStages I_Stages, UInt32 I_Count = 1) noexcept
        { return { I_Index, ERHISetBindingType::UniformBuffer, I_Count, I_Stages }; }

        static constexpr [[nodiscard]] FRHISetBinding
        StorageBuffer(UInt32 I_Index, ERHIShaderStages I_Stages, UInt32 I_Count = 1) noexcept
        { return { I_Index, ERHISetBindingType::StorageBuffer, I_Count, I_Stages }; }

        static constexpr [[nodiscard]] FRHISetBinding
        CombinedImageSampler(UInt32 I_Index, ERHIShaderStages I_Stages, UInt32 I_Count = 1, UInt64 immutableSamplerID = 0) noexcept
        { return { I_Index, ERHISetBindingType::CombinedImageSampler, I_Count, I_Stages, immutableSamplerID }; }

        [[nodiscard]] constexpr Bool
        IsImmutableSampler() const noexcept
        { return (Type == ERHISetBindingType::CombinedImageSampler) && ImmutableSamplerID != 0; }

        [[nodiscard]] constexpr Bool
        IsValid() const noexcept
        {
            return Type         != ERHISetBindingType::Undefined &&
                   Count        != 0                             &&
                   ShaderStages != ERHIShaderStages::Undefined;
        }

        friend constexpr Bool
        operator==(const FRHISetBinding& I_A, const FRHISetBinding& I_B) noexcept = default;

        friend constexpr auto operator<=>(const FRHISetBinding& a,
                                          const FRHISetBinding& b) noexcept
        {
            if (a.Index != b.Index) return a.Index <=> b.Index;
            if (a.Type  != b.Type)  return a.Type  <=> b.Type;
            if (a.Count != b.Count) return a.Count <=> b.Count;
            if (ToUnderlying(a.ShaderStages) != ToUnderlying(b.ShaderStages))
                return ToUnderlying(a.ShaderStages) <=> ToUnderlying(b.ShaderStages);
            return a.ImmutableSamplerID <=> b.ImmutableSamplerID;
        }

        [[nodiscard]] UInt64
        Hash() const noexcept
        {
            UInt64 Seed = 0;
            return GoldenRatioHashCombine(0,
                Index,
                Count,
                ShaderStages,
                ImmutableSamplerID);
        }
    };

    struct VISERA_RUNTIME_API FRHISetLayout
    {
        TArray<FRHISetBinding> Bindings;
    };
}