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
        R8G8B8_sRGB         = vk::Format::eR8G8B8Srgb,
        R8G8B8_UNorm        = vk::Format::eR8G8B8Unorm,

        B8G8R8_sRGB         = vk::Format::eB8G8R8Srgb,
        B8G8R8_UNorm        = vk::Format::eB8G8R8Unorm,

        R8G8B8A8_sRGB       = vk::Format::eR8G8B8A8Srgb,
        R8G8B8A8_UNorm      = vk::Format::eR8G8B8A8Unorm,

        B8G8R8A8_sRGB       = vk::Format::eB8G8R8A8Srgb,
        B8G8R8A8_UNorm      = vk::Format::eB8G8R8A8Unorm,

        Vector2F            = vk::Format::eR32G32Sfloat,
        Vector3F            = vk::Format::eR32G32B32Sfloat,
        Vector4F            = vk::Format::eR32G32B32A32Sfloat,

        Undefined,
    };
    [[nodiscard]] constexpr vk::Format
    TypeCast(ERHIFormat I_Format) { return static_cast<vk::Format>(I_Format); }

    enum class ERHIDescriptorType
    {
        CombinedImageSampler    = vk::DescriptorType::eCombinedImageSampler,
        UniformBuffer           = vk::DescriptorType::eUniformBuffer,
        StorageBuffer           = vk::DescriptorType::eStorageBuffer,

        Undefined,
    };
    [[nodiscard]] constexpr vk::DescriptorType
    TypeCast(ERHIDescriptorType I_DescriptorType) { return static_cast<vk::DescriptorType>(I_DescriptorType); }

    enum class ERHIShaderStages : UInt32
    {
        Vertex    = vk::ShaderStageFlagBits::eVertex,
        Fragment  = vk::ShaderStageFlagBits::eFragment,
        All       = vk::ShaderStageFlagBits::eAll,

        Undefined,
    };
    VISERA_MAKE_FLAGS(ERHIShaderStages);
    [[nodiscard]] constexpr vk::ShaderStageFlags
    TypeCast(ERHIShaderStages I_ShaderStages) { return static_cast<vk::ShaderStageFlagBits>(I_ShaderStages); }

    enum class ERHISamplerType
    {
        Linear,
        Nearest,
    };

    enum class ERHIAttachmentLoadOp
    {
        Load     = vk::AttachmentLoadOp::eLoad,
        Clear    = vk::AttachmentLoadOp::eClear,
        Whatever = vk::AttachmentLoadOp::eDontCare,
    };
    [[nodiscard]] constexpr vk::AttachmentLoadOp
    TypeCast(ERHIAttachmentLoadOp I_AttachmentLoadOp) { return static_cast<vk::AttachmentLoadOp>(I_AttachmentLoadOp); }

    enum class ERHIPrimitiveTopology
    {
        PointList    = vk::PrimitiveTopology::ePointList,
        LineList     = vk::PrimitiveTopology::eLineList,
        LineStrip    = vk::PrimitiveTopology::eLineStrip,
        TriangleList = vk::PrimitiveTopology::eTriangleList,

        Undefined,
    };
    [[nodiscard]] constexpr vk::PrimitiveTopology
    TypeCast(ERHIPrimitiveTopology I_PrimitiveTopology) { return static_cast<vk::PrimitiveTopology>(I_PrimitiveTopology); }

    enum class ERHIPolygonMode
    {
        Fill         = vk::PolygonMode::eFill,
        Line         = vk::PolygonMode::eLine,
        Point        = vk::PolygonMode::ePoint,

        Undefined,
    };
    [[nodiscard]] constexpr vk::PolygonMode
    TypeCast(ERHIPolygonMode I_PolygonMode) { return static_cast<vk::PolygonMode>(I_PolygonMode); }

    enum class ERHICullMode
    {
        None        = vk::CullModeFlagBits::eNone,
        Front       = vk::CullModeFlagBits::eFront,
        Back        = vk::CullModeFlagBits::eBack,
        TwoSided    = vk::CullModeFlagBits::eFrontAndBack,
    };
    [[nodiscard]] constexpr vk::CullModeFlags
    TypeCast(ERHICullMode I_CullMode) { return static_cast<vk::CullModeFlagBits>(I_CullMode); }

    enum class ERHISamplingRate
    {
        X1 = vk::SampleCountFlagBits::e1,
        X2 = vk::SampleCountFlagBits::e2,
        X4 = vk::SampleCountFlagBits::e4,
        X8 = vk::SampleCountFlagBits::e8,
    };
    [[nodiscard]] constexpr vk::SampleCountFlags
    TypeCast(ERHISamplingRate I_SamplingRate) { return static_cast<vk::SampleCountFlagBits>(I_SamplingRate); }

    enum class ERHIBlendOp
    {
        Add  = vk::BlendOp::eAdd,

        None,
    };
    [[nodiscard]] constexpr vk::BlendOp
    TypeCast(ERHIBlendOp I_BlendOp) { return static_cast<vk::BlendOp>(I_BlendOp); }
}

VISERA_MAKE_FORMATTER(Visera::ERHIShaderStages,
    const char* StageName = "Undefined";
    switch (I_Formatee)
    {
        case Visera::ERHIShaderStages::Vertex:   StageName = "Vertex";   break;
        case Visera::ERHIShaderStages::Fragment: StageName = "Fragment"; break;
        default: break;
    },
    "{}", StageName
);