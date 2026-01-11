module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Common;
#define VISERA_MODULE_NAME "RHI.Common"
export import Visera.Core.Traits.Flags;
       import Visera.Core.Types.Handle;
       import Visera.Global.Log;
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

    enum class ERHIFilter : UInt8
    {
        Nearest = static_cast<UInt8>(vk::Filter::eNearest),
        Linear  = static_cast<UInt8>(vk::Filter::eLinear),
    };
    [[nodiscard]] constexpr vk::Filter
    TypeCast(ERHIFilter I_Filter) { return static_cast<vk::Filter>(I_Filter); }

    enum class ERHISamplerAddressMode : UInt8
    {
        Repeat              = static_cast<UInt8>(vk::SamplerAddressMode::eRepeat),
        MirroredRepeat      = static_cast<UInt8>(vk::SamplerAddressMode::eMirroredRepeat),
        ClampToEdge         = static_cast<UInt8>(vk::SamplerAddressMode::eClampToEdge),
        ClampToBorder       = static_cast<UInt8>(vk::SamplerAddressMode::eClampToBorder),
        MirrorClampToEdge   = static_cast<UInt8>(vk::SamplerAddressMode::eMirrorClampToEdge),
    };
    [[nodiscard]] constexpr vk::SamplerAddressMode
    TypeCast(ERHISamplerAddressMode I_AddressMode) { return static_cast<vk::SamplerAddressMode>(I_AddressMode); }

    enum class ERHIImageViewType : UInt8
    {
        Image1D         = static_cast<UInt8>(vk::ImageViewType::e1D),
        Image2D         = static_cast<UInt8>(vk::ImageViewType::e2D),
        Image3D         = static_cast<UInt8>(vk::ImageViewType::e3D),
        Image1DArray    = static_cast<UInt8>(vk::ImageViewType::e1DArray),
        Image2DArray    = static_cast<UInt8>(vk::ImageViewType::e2DArray),
        Cube            = static_cast<UInt8>(vk::ImageViewType::eCube),
        CubeArray       = static_cast<UInt8>(vk::ImageViewType::eCubeArray),
    };
    [[nodiscard]] constexpr vk::ImageViewType
    TypeCast(ERHIImageViewType I_ImageViewType) { return static_cast<vk::ImageViewType>(I_ImageViewType); }

    enum class ERHIImageAspect : UInt32
    {
        None         = 0,
        Color        = static_cast<UInt32>(vk::ImageAspectFlagBits::eColor),
        Depth        = static_cast<UInt32>(vk::ImageAspectFlagBits::eDepth),
        Stencil      = static_cast<UInt32>(vk::ImageAspectFlagBits::eStencil),
        DepthStencil = static_cast<UInt32>(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil),
    };
    VISERA_MAKE_FLAGS(ERHIImageAspect);
    [[nodiscard]] constexpr vk::ImageAspectFlags
    TypeCast(ERHIImageAspect I_ImageAspect) { return static_cast<vk::ImageAspectFlagBits>(I_ImageAspect); }

    struct FRHIExtent3D
    {
        UInt32 Width  = 0;
        UInt32 Height = 0;
        UInt32 Depth  = 0;
    };
    [[nodiscard]] inline constexpr vk::Extent3D
    TypeCast(const FRHIExtent3D& I_Extent) { return vk::Extent3D{I_Extent.Width, I_Extent.Height, I_Extent.Depth}; }

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

    enum class ERHIImageType : UInt8
    {
        Image1D = static_cast<UInt8>(vk::ImageType::e1D),
        Image2D = static_cast<UInt8>(vk::ImageType::e2D),
        Image3D = static_cast<UInt8>(vk::ImageType::e3D),

        Undefined
    };
    [[nodiscard]] constexpr vk::ImageType
    TypeCast(ERHIImageType I_TextureType) { return static_cast<vk::ImageType>(I_TextureType); }

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

VISERA_MAKE_FORMATTER(Visera::ERHIDescriptorType,
    const char* DescriptorTypeName = "Undefined";
    switch (I_Formatee)
    {
        case Visera::ERHIDescriptorType::CombinedImageSampler: DescriptorTypeName = "CombinedImageSampler"; break;
        case Visera::ERHIDescriptorType::StorageImage:        DescriptorTypeName = "StorageImage";        break;
        case Visera::ERHIDescriptorType::UniformBuffer:       DescriptorTypeName = "UniformBuffer";       break;
        case Visera::ERHIDescriptorType::StorageBuffer:       DescriptorTypeName = "StorageBuffer";       break;
        case Visera::ERHIDescriptorType::Undefined:           DescriptorTypeName = "Undefined";           break;
        default: break;
    },
    "{}", DescriptorTypeName
);

VISERA_MAKE_FORMATTER(Visera::ERHIShaderStages,
    const char* StageName = "Undefined";
    switch (I_Formatee)
    {
        case Visera::ERHIShaderStages::Vertex:    StageName = "Vertex";   break;
        case Visera::ERHIShaderStages::Fragment:  StageName = "Fragment"; break;
        case Visera::ERHIShaderStages::Compute:   StageName = "Compute";  break;
        case Visera::ERHIShaderStages::All:       StageName = "All";      break;
        case Visera::ERHIShaderStages::Undefined: StageName = "Undefined"; break;
        default: break;
    },
    "{}", StageName
);

VISERA_MAKE_FORMATTER(Visera::ERHISamplerType,
    const char* SamplerTypeName = "Unknown";
    switch (I_Formatee)
    {
        case Visera::ERHISamplerType::Linear:   SamplerTypeName = "Linear";   break;
        case Visera::ERHISamplerType::Nearest:  SamplerTypeName = "Nearest";  break;
        default: break;
    },
    "{}", SamplerTypeName
);

VISERA_MAKE_FORMATTER(Visera::ERHIFilter,
    const char* FilterName = "Unknown";
    switch (I_Formatee)
    {
        case Visera::ERHIFilter::Nearest: FilterName = "Nearest"; break;
        case Visera::ERHIFilter::Linear:  FilterName = "Linear";  break;
        default: break;
    },
    "{}", FilterName
);

VISERA_MAKE_FORMATTER(Visera::ERHISamplerAddressMode,
    const char* AddressModeName = "Unknown";
    switch (I_Formatee)
    {
        case Visera::ERHISamplerAddressMode::Repeat:             AddressModeName = "Repeat";            break;
        case Visera::ERHISamplerAddressMode::MirroredRepeat:     AddressModeName = "MirroredRepeat";     break;
        case Visera::ERHISamplerAddressMode::ClampToEdge:        AddressModeName = "ClampToEdge";       break;
        case Visera::ERHISamplerAddressMode::ClampToBorder:      AddressModeName = "ClampToBorder";      break;
        case Visera::ERHISamplerAddressMode::MirrorClampToEdge:  AddressModeName = "MirrorClampToEdge";  break;
        default: break;
    },
    "{}", AddressModeName
);

VISERA_MAKE_FORMATTER(Visera::ERHIImageViewType,
    const char* ImageViewTypeName = "Unknown";
    switch (I_Formatee)
    {
        case Visera::ERHIImageViewType::Image1D:      ImageViewTypeName = "Image1D";      break;
        case Visera::ERHIImageViewType::Image2D:      ImageViewTypeName = "Image2D";      break;
        case Visera::ERHIImageViewType::Image3D:      ImageViewTypeName = "Image3D";      break;
        case Visera::ERHIImageViewType::Image1DArray: ImageViewTypeName = "Image1DArray"; break;
        case Visera::ERHIImageViewType::Image2DArray: ImageViewTypeName = "Image2DArray"; break;
        case Visera::ERHIImageViewType::Cube:         ImageViewTypeName = "Cube";         break;
        case Visera::ERHIImageViewType::CubeArray:    ImageViewTypeName = "CubeArray";    break;
        default: break;
    },
    "{}", ImageViewTypeName
);

VISERA_MAKE_FORMATTER(Visera::ERHIImageAspect,
    const char* ImageAspectName = "None";
    switch (I_Formatee)
    {
        case Visera::ERHIImageAspect::None:         ImageAspectName = "None";         break;
        case Visera::ERHIImageAspect::Color:        ImageAspectName = "Color";        break;
        case Visera::ERHIImageAspect::Depth:        ImageAspectName = "Depth";        break;
        case Visera::ERHIImageAspect::Stencil:      ImageAspectName = "Stencil";      break;
        case Visera::ERHIImageAspect::DepthStencil: ImageAspectName = "DepthStencil"; break;
        default: break;
    },
    "{}", ImageAspectName
);

VISERA_MAKE_FORMATTER(Visera::ERHIImageType,
    const char* ImageTypeName = "Undefined";
    switch (I_Formatee)
    {
        case Visera::ERHIImageType::Image1D:    ImageTypeName = "Image1D";    break;
        case Visera::ERHIImageType::Image2D:    ImageTypeName = "Image2D";    break;
        case Visera::ERHIImageType::Image3D:    ImageTypeName = "Image3D";    break;
        case Visera::ERHIImageType::Undefined:  ImageTypeName = "Undefined"; break;
        default: break;
    },
    "{}", ImageTypeName
);

VISERA_MAKE_FORMATTER(Visera::ERHIAttachmentLoadOp,
    const char* LoadOpName = "Unknown";
    switch (I_Formatee)
    {
        case Visera::ERHIAttachmentLoadOp::Load:      LoadOpName = "Load";     break;
        case Visera::ERHIAttachmentLoadOp::Clear:     LoadOpName = "Clear";    break;
        case Visera::ERHIAttachmentLoadOp::Whatever:  LoadOpName = "Whatever"; break;
        default: break;
    },
    "{}", LoadOpName
);

VISERA_MAKE_FORMATTER(Visera::ERHIPrimitiveTopology,
    const char* TopologyName = "Undefined";
    switch (I_Formatee)
    {
        case Visera::ERHIPrimitiveTopology::PointList:    TopologyName = "PointList";    break;
        case Visera::ERHIPrimitiveTopology::LineList:     TopologyName = "LineList";     break;
        case Visera::ERHIPrimitiveTopology::LineStrip:    TopologyName = "LineStrip";    break;
        case Visera::ERHIPrimitiveTopology::TriangleList: TopologyName = "TriangleList"; break;
        case Visera::ERHIPrimitiveTopology::Undefined:    TopologyName = "Undefined";    break;
        default: break;
    },
    "{}", TopologyName
);

VISERA_MAKE_FORMATTER(Visera::ERHIPolygonMode,
    const char* PolygonModeName = "Undefined";
    switch (I_Formatee)
    {
        case Visera::ERHIPolygonMode::Fill:      PolygonModeName = "Fill";      break;
        case Visera::ERHIPolygonMode::Line:      PolygonModeName = "Line";      break;
        case Visera::ERHIPolygonMode::Point:     PolygonModeName = "Point";     break;
        case Visera::ERHIPolygonMode::Undefined: PolygonModeName = "Undefined"; break;
        default: break;
    },
    "{}", PolygonModeName
);

VISERA_MAKE_FORMATTER(Visera::ERHICullMode,
    const char* CullModeName = "Unknown";
    switch (I_Formatee)
    {
        case Visera::ERHICullMode::None:     CullModeName = "None";     break;
        case Visera::ERHICullMode::Front:    CullModeName = "Front";    break;
        case Visera::ERHICullMode::Back:     CullModeName = "Back";     break;
        case Visera::ERHICullMode::TwoSided: CullModeName = "TwoSided"; break;
        default: break;
    },
    "{}", CullModeName
);

VISERA_MAKE_FORMATTER(Visera::ERHISamplingRate,
    const char* SamplingRateName = "Unknown";
    switch (I_Formatee)
    {
        case Visera::ERHISamplingRate::X1: SamplingRateName = "X1"; break;
        case Visera::ERHISamplingRate::X2: SamplingRateName = "X2"; break;
        case Visera::ERHISamplingRate::X4: SamplingRateName = "X4"; break;
        case Visera::ERHISamplingRate::X8: SamplingRateName = "X8"; break;
        default: break;
    },
    "{}", SamplingRateName
);

VISERA_MAKE_FORMATTER(Visera::ERHIBlendOp,
    const char* BlendOpName = "None";
    switch (I_Formatee)
    {
        case Visera::ERHIBlendOp::Add:  BlendOpName = "Add";  break;
        case Visera::ERHIBlendOp::None: BlendOpName = "None"; break;
        default: break;
    },
    "{}", BlendOpName
);

VISERA_MAKE_FORMATTER(Visera::ERHIImageUsage,
    const char* ImageUsageName = "None";
    switch (I_Formatee)
    {
        case Visera::ERHIImageUsage::None:            ImageUsageName = "None";            break;
        case Visera::ERHIImageUsage::ShaderResource:  ImageUsageName = "ShaderResource";  break;
        case Visera::ERHIImageUsage::RenderTarget:    ImageUsageName = "RenderTarget";    break;
        case Visera::ERHIImageUsage::DepthStencil:    ImageUsageName = "DepthStencil";    break;
        case Visera::ERHIImageUsage::UnorderedAccess: ImageUsageName = "UnorderedAccess"; break;
        case Visera::ERHIImageUsage::TransferSrc:     ImageUsageName = "TransferSrc";     break;
        case Visera::ERHIImageUsage::TransferDst:     ImageUsageName = "TransferDst";     break;
        default: break;
    },
    "{}", ImageUsageName
);

VISERA_MAKE_FORMATTER(Visera::ERHIBufferUsage,
    const char* BufferUsageName = "None";
    switch (I_Formatee)
    {
        case Visera::ERHIBufferUsage::None:          BufferUsageName = "None";         break;
        case Visera::ERHIBufferUsage::VertexBuffer:  BufferUsageName = "VertexBuffer";  break;
        case Visera::ERHIBufferUsage::IndexBuffer:   BufferUsageName = "IndexBuffer";   break;
        case Visera::ERHIBufferUsage::UniformBuffer: BufferUsageName = "UniformBuffer"; break;
        case Visera::ERHIBufferUsage::StorageBuffer: BufferUsageName = "StorageBuffer"; break;
        case Visera::ERHIBufferUsage::TransferSrc:   BufferUsageName = "TransferSrc";  break;
        case Visera::ERHIBufferUsage::TransferDst:   BufferUsageName = "TransferDst";   break;
        default: break;
    },
    "{}", BufferUsageName
);