module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan.hpp>
export module Visera.Runtime.RHI.Common;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.Traits.Flags;
import Visera.Core.Log;

export namespace Visera
{
    enum class ERHIFormat
    {
        Undefined           = 0,
        R8G8B8_sRGB         = vk::Format::eR8G8B8Srgb,
        R8G8B8_UNorm        = vk::Format::eR8G8B8Unorm,

        B8G8R8_sRGB         = vk::Format::eB8G8R8Srgb,
        B8G8R8_UNorm        = vk::Format::eB8G8R8Unorm,

        R8G8B8A8_sRGB       = vk::Format::eR8G8B8A8Srgb,
        R8G8B8A8_UNorm      = vk::Format::eR8G8B8A8Unorm,

        B8G8R8A8_sRGB       = vk::Format::eB8G8R8A8Srgb,
        B8G8R8A8_UNorm      = vk::Format::eB8G8R8A8Unorm,

        Vector4F            = vk::Format::eR32G32B32A32Sfloat,
    };
    [[nodiscard]] constexpr vk::Format
    TypeCast(ERHIFormat I_RHIFormat) { return static_cast<vk::Format>(I_RHIFormat); }

    enum class ERHIDescriptorType
    {
        Undefined               = 0,
        CombinedImageSampler    = vk::DescriptorType::eCombinedImageSampler,
        UniformBuffer           = vk::DescriptorType::eUniformBuffer,
        StorageBuffer           = vk::DescriptorType::eStorageBuffer,
    };
    [[nodiscard]] constexpr vk::DescriptorType
    TypeCast(ERHIDescriptorType I_RHIDescriptorType) { return static_cast<vk::DescriptorType>(I_RHIDescriptorType); }

    enum class ERHIShaderStages : UInt32
    {
        Undefined = 0,
        Vertex    = vk::ShaderStageFlagBits::eVertex,
        Fragment  = vk::ShaderStageFlagBits::eFragment,
        All       = vk::ShaderStageFlagBits::eAll,
    };
    VISERA_MAKE_FLAGS(ERHIShaderStages);
    [[nodiscard]] constexpr vk::ShaderStageFlags
    TypeCast(ERHIShaderStages I_RHIShaderStages) { return static_cast<vk::ShaderStageFlagBits>(I_RHIShaderStages); }

    enum class ERHISamplerType
    {
        Linear,
        Nearest,
    };
}