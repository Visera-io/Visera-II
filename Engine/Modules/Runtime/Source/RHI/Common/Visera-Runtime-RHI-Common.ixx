module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Common;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.Traits.Flags;
import Visera.Runtime.Log;
import vulkan_hpp;

export namespace Visera
{
    enum class ERHIFormat : UInt32
    {
        R8G8B8_sRGB         = static_cast<UInt32>(vk::Format::eR8G8B8Srgb),
        R8G8B8_UNorm        = static_cast<UInt32>(vk::Format::eR8G8B8Unorm),

        B8G8R8_sRGB         = static_cast<UInt32>(vk::Format::eB8G8R8Srgb),
        B8G8R8_UNorm        = static_cast<UInt32>(vk::Format::eB8G8R8Unorm),

        R8G8B8A8_sRGB       = static_cast<UInt32>(vk::Format::eR8G8B8A8Srgb),
        R8G8B8A8_UNorm      = static_cast<UInt32>(vk::Format::eR8G8B8A8Unorm),

        B8G8R8A8_sRGB       = static_cast<UInt32>(vk::Format::eB8G8R8A8Srgb),
        B8G8R8A8_UNorm      = static_cast<UInt32>(vk::Format::eB8G8R8A8Unorm),

        Vector2F            = static_cast<UInt32>(vk::Format::eR32G32Sfloat),
        Vector3F            = static_cast<UInt32>(vk::Format::eR32G32B32Sfloat),
        Vector4F            = static_cast<UInt32>(vk::Format::eR32G32B32A32Sfloat),

        Undefined,
    };
    [[nodiscard]] constexpr vk::Format
    TypeCast(ERHIFormat I_Format) { return static_cast<vk::Format>(I_Format); }

    enum class ERHIDescriptorType : UInt32
    {
        CombinedImageSampler    = static_cast<UInt32>(vk::DescriptorType::eCombinedImageSampler),
        UniformBuffer           = static_cast<UInt32>(vk::DescriptorType::eUniformBuffer),
        StorageBuffer           = static_cast<UInt32>(vk::DescriptorType::eStorageBuffer),

        Undefined,
    };
    [[nodiscard]] constexpr vk::DescriptorType
    TypeCast(ERHIDescriptorType I_DescriptorType) { return static_cast<vk::DescriptorType>(I_DescriptorType); }

    enum class ERHIShaderStages : UInt32
    {
        Vertex    = static_cast<UInt32>(vk::ShaderStageFlagBits::eVertex),
        Fragment  = static_cast<UInt32>(vk::ShaderStageFlagBits::eFragment),
        All       = static_cast<UInt32>(vk::ShaderStageFlagBits::eAll),

        Undefined,
    };
    VISERA_MAKE_FLAGS(ERHIShaderStages);
    [[nodiscard]] constexpr vk::ShaderStageFlags
    TypeCast(ERHIShaderStages I_ShaderStages) { return static_cast<vk::ShaderStageFlagBits>(I_ShaderStages); }

    enum class ERHISamplerType : UInt32
    {
        Linear,
        Nearest,
    };

    enum class ERHIAttachmentLoadOp : UInt32
    {
        Load     = static_cast<UInt32>(vk::AttachmentLoadOp::eLoad),
        Clear    = static_cast<UInt32>(vk::AttachmentLoadOp::eClear),
        Whatever = static_cast<UInt32>(vk::AttachmentLoadOp::eDontCare),
    };
    [[nodiscard]] constexpr vk::AttachmentLoadOp
    TypeCast(ERHIAttachmentLoadOp I_AttachmentLoadOp) { return static_cast<vk::AttachmentLoadOp>(I_AttachmentLoadOp); }

    enum class ERHIPrimitiveTopology
    {
        PointList    = static_cast<UInt32>(vk::PrimitiveTopology::ePointList),
        LineList     = static_cast<UInt32>(vk::PrimitiveTopology::eLineList),
        LineStrip    = static_cast<UInt32>(vk::PrimitiveTopology::eLineStrip),
        TriangleList = static_cast<UInt32>(vk::PrimitiveTopology::eTriangleList),

        Undefined,
    };
    [[nodiscard]] constexpr vk::PrimitiveTopology
    TypeCast(ERHIPrimitiveTopology I_PrimitiveTopology) { return static_cast<vk::PrimitiveTopology>(I_PrimitiveTopology); }

    enum class ERHIPolygonMode
    {
        Fill         = static_cast<UInt32>(vk::PolygonMode::eFill),
        Line         = static_cast<UInt32>(vk::PolygonMode::eLine),
        Point        = static_cast<UInt32>(vk::PolygonMode::ePoint),

        Undefined,
    };
    [[nodiscard]] constexpr vk::PolygonMode
    TypeCast(ERHIPolygonMode I_PolygonMode) { return static_cast<vk::PolygonMode>(I_PolygonMode); }

    enum class ERHICullMode
    {
        None        = static_cast<UInt32>(vk::CullModeFlagBits::eNone),
        Front       = static_cast<UInt32>(vk::CullModeFlagBits::eFront),
        Back        = static_cast<UInt32>(vk::CullModeFlagBits::eBack),
        TwoSided    = static_cast<UInt32>(vk::CullModeFlagBits::eFrontAndBack),
    };
    [[nodiscard]] constexpr vk::CullModeFlags
    TypeCast(ERHICullMode I_CullMode) { return static_cast<vk::CullModeFlagBits>(I_CullMode); }

    enum class ERHISamplingRate
    {
        X1 = static_cast<UInt32>(vk::SampleCountFlagBits::e1),
        X2 = static_cast<UInt32>(vk::SampleCountFlagBits::e2),
        X4 = static_cast<UInt32>(vk::SampleCountFlagBits::e4),
        X8 = static_cast<UInt32>(vk::SampleCountFlagBits::e8),
    };
    [[nodiscard]] constexpr vk::SampleCountFlags
    TypeCast(ERHISamplingRate I_SamplingRate) { return static_cast<vk::SampleCountFlagBits>(I_SamplingRate); }

    enum class ERHIBlendOp
    {
        Add  = static_cast<UInt32>(vk::BlendOp::eAdd),

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