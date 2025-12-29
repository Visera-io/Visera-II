module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Common;
#define VISERA_MODULE_NAME "RHI.Common"
export import Visera.Core.Traits.Flags;
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


        R16G16B16A16_Float  = static_cast<UInt32>(vk::Format::eR16G16B16A16Sfloat),
        R32G32_Float        = static_cast<UInt32>(vk::Format::eR32G32Sfloat),
        R32G32B32_Float     = static_cast<UInt32>(vk::Format::eR32G32B32Sfloat),
        R32G32B32A32_Float  = static_cast<UInt32>(vk::Format::eR32G32B32A32Sfloat),

        Vector2F            = R32G32_Float,
        Vector3F            = R32G32B32_Float,
        Vector4F            = R32G32B32A32_Float,

        Undefined           = static_cast<UInt32>(vk::Format::eUndefined),
    };
    [[nodiscard]] constexpr vk::Format
    TypeCast(ERHIFormat I_Format) { return static_cast<vk::Format>(I_Format); }

    enum class ERHIDescriptorType : UInt32
    {
        CombinedImageSampler    = static_cast<UInt32>(vk::DescriptorType::eCombinedImageSampler),
        StorageImage            = static_cast<UInt32>(vk::DescriptorType::eStorageImage),
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
        Compute   = static_cast<UInt32>(vk::ShaderStageFlagBits::eCompute),
        All       = static_cast<UInt32>(vk::ShaderStageFlagBits::eAll),

        Undefined,
    };
    VISERA_MAKE_FLAGS(ERHIShaderStages);
    [[nodiscard]] constexpr vk::ShaderStageFlags
    TypeCast(ERHIShaderStages I_ShaderStages) { return static_cast<vk::ShaderStageFlagBits>(I_ShaderStages); }

    enum class ERHISamplerType : UInt8
    {
        Linear,
        Nearest,
    };

    enum class ERHIAttachmentLoadOp : UInt8
    {
        Load     = static_cast<UInt8>(vk::AttachmentLoadOp::eLoad),
        Clear    = static_cast<UInt8>(vk::AttachmentLoadOp::eClear),
        Whatever = static_cast<UInt8>(vk::AttachmentLoadOp::eDontCare),
    };
    [[nodiscard]] constexpr vk::AttachmentLoadOp
    TypeCast(ERHIAttachmentLoadOp I_AttachmentLoadOp) { return static_cast<vk::AttachmentLoadOp>(I_AttachmentLoadOp); }

    enum class ERHIPrimitiveTopology : UInt8
    {
        PointList    = static_cast<UInt8>(vk::PrimitiveTopology::ePointList),
        LineList     = static_cast<UInt8>(vk::PrimitiveTopology::eLineList),
        LineStrip    = static_cast<UInt8>(vk::PrimitiveTopology::eLineStrip),
        TriangleList = static_cast<UInt8>(vk::PrimitiveTopology::eTriangleList),

        Undefined,
    };
    [[nodiscard]] constexpr vk::PrimitiveTopology
    TypeCast(ERHIPrimitiveTopology I_PrimitiveTopology) { return static_cast<vk::PrimitiveTopology>(I_PrimitiveTopology); }

    enum class ERHIPolygonMode : UInt8
    {
        Fill         = static_cast<UInt8>(vk::PolygonMode::eFill),
        Line         = static_cast<UInt8>(vk::PolygonMode::eLine),
        Point        = static_cast<UInt8>(vk::PolygonMode::ePoint),

        Undefined,
    };
    [[nodiscard]] constexpr vk::PolygonMode
    TypeCast(ERHIPolygonMode I_PolygonMode) { return static_cast<vk::PolygonMode>(I_PolygonMode); }

    enum class ERHICullMode : UInt8
    {
        None        = static_cast<UInt8>(vk::CullModeFlagBits::eNone),
        Front       = static_cast<UInt8>(vk::CullModeFlagBits::eFront),
        Back        = static_cast<UInt8>(vk::CullModeFlagBits::eBack),
        TwoSided    = static_cast<UInt8>(vk::CullModeFlagBits::eFrontAndBack),
    };
    [[nodiscard]] constexpr vk::CullModeFlags
    TypeCast(ERHICullMode I_CullMode) { return static_cast<vk::CullModeFlagBits>(I_CullMode); }

    enum class ERHISamplingRate : UInt8
    {
        X1 = static_cast<UInt8>(vk::SampleCountFlagBits::e1),
        X2 = static_cast<UInt8>(vk::SampleCountFlagBits::e2),
        X4 = static_cast<UInt8>(vk::SampleCountFlagBits::e4),
        X8 = static_cast<UInt8>(vk::SampleCountFlagBits::e8),
    };
    [[nodiscard]] constexpr vk::SampleCountFlags
    TypeCast(ERHISamplingRate I_SamplingRate) { return static_cast<vk::SampleCountFlagBits>(I_SamplingRate); }

    enum class ERHIBlendOp : UInt8
    {
        Add  = static_cast<UInt8>(vk::BlendOp::eAdd),

        None,
    };
    [[nodiscard]] constexpr vk::BlendOp
    TypeCast(ERHIBlendOp I_BlendOp) { return static_cast<vk::BlendOp>(I_BlendOp); }

    enum class ERHITextureType : UInt8
    {
        Texture1D = static_cast<UInt8>(vk::ImageType::e1D),
        Texture2D = static_cast<UInt8>(vk::ImageType::e2D),
        Texture3D = static_cast<UInt8>(vk::ImageType::e3D),

        Undefined
    };
    [[nodiscard]] constexpr vk::ImageType
    TypeCast(ERHITextureType I_TextureType) { return static_cast<vk::ImageType>(I_TextureType); }

    enum class ERHIImageUsage : UInt32
    {
        None            = 0,
        ShaderResource  = static_cast<UInt32>(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eInputAttachment),
        RenderTarget    = static_cast<UInt32>(vk::ImageUsageFlagBits::eColorAttachment),
        DepthStencil    = static_cast<UInt32>(vk::ImageUsageFlagBits::eDepthStencilAttachment),
        UnorderedAccess = static_cast<UInt32>(vk::ImageUsageFlagBits::eStorage),
        TransferSrc     = static_cast<UInt32>(vk::ImageUsageFlagBits::eTransferSrc),
        TransferDst     = static_cast<UInt32>(vk::ImageUsageFlagBits::eTransferDst),
    };
    VISERA_MAKE_FLAGS(ERHIImageUsage);
    [[nodiscard]] constexpr vk::ImageUsageFlags
    TypeCast(ERHIImageUsage I_TextureUsage) { return static_cast<vk::ImageUsageFlagBits>(I_TextureUsage); }

    enum class ERHIBufferUsage : UInt32
    {
        None = 0,
        VertexBuffer    = static_cast<UInt32>(vk::BufferUsageFlagBits2::eVertexBuffer),
        IndexBuffer     = static_cast<UInt32>(vk::BufferUsageFlagBits2::eIndexBuffer),
        UniformBuffer   = static_cast<UInt32>(vk::BufferUsageFlagBits2::eUniformBuffer),
        StorageBuffer   = static_cast<UInt32>(vk::BufferUsageFlagBits2::eStorageBuffer),
        TransferSrc     = static_cast<UInt32>(vk::BufferUsageFlagBits2::eTransferSrc),
        TransferDst     = static_cast<UInt32>(vk::BufferUsageFlagBits2::eTransferDst),
    };
    VISERA_MAKE_FLAGS(ERHIBufferUsage);
    [[nodiscard]] constexpr vk::BufferUsageFlagBits2
    TypeCast(ERHIBufferUsage I_BufferUsage) { return static_cast<vk::BufferUsageFlagBits2>(I_BufferUsage); }
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

VISERA_MAKE_FORMATTER(Visera::ERHIFormat,
    const char* FormatName = "Undefined";
    switch (I_Formatee)
    {
        case Visera::ERHIFormat::R8G8B8_sRGB:        FormatName = "R8G8B8_sRGB";        break;
        case Visera::ERHIFormat::R8G8B8_UNorm:       FormatName = "R8G8B8_UNorm";       break;

        case Visera::ERHIFormat::B8G8R8_sRGB:        FormatName = "B8G8R8_sRGB";        break;
        case Visera::ERHIFormat::B8G8R8_UNorm:       FormatName = "B8G8R8_UNorm";       break;

        case Visera::ERHIFormat::R8G8B8A8_sRGB:      FormatName = "R8G8B8A8_sRGB";      break;
        case Visera::ERHIFormat::R8G8B8A8_UNorm:     FormatName = "R8G8B8A8_UNorm";     break;

        case Visera::ERHIFormat::B8G8R8A8_sRGB:      FormatName = "B8G8R8A8_sRGB";      break;
        case Visera::ERHIFormat::B8G8R8A8_UNorm:     FormatName = "B8G8R8A8_UNorm";     break;

        case Visera::ERHIFormat::R16G16B16A16_Float: FormatName = "R16G16B16A16_Float"; break;
        case Visera::ERHIFormat::R32G32_Float:       FormatName = "R32G32_Float";       break;
        case Visera::ERHIFormat::R32G32B32_Float:    FormatName = "R32G32B32_Float";    break;
        case Visera::ERHIFormat::R32G32B32A32_Float: FormatName = "R32G32B32A32_Float"; break;

        case Visera::ERHIFormat::Undefined:          FormatName = "Undefined";          break;
        default: break;
    },
    "{}", FormatName
);