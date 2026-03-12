module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.Resource.RenderPass;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Common;
import Visera.Runtime.RHI.Registry.Item;
import Visera.Runtime.RHI.Registry.Handle;
import Visera.Runtime.RHI.Vulkan.Pipeline.Render;
import Visera.Core.Math.Color.Linear;
import Visera.Core.Containers.Array;

export namespace Visera
{
    struct FRHIColorAttachmentDesc
    {
        FRHITextureHandle      Texture;
        ERHIAttachmentLoadOp   LoadOp           { ERHIAttachmentLoadOp::Clear };
        ERHIAttachmentStoreOp  StoreOp          { ERHIAttachmentStoreOp::Store };
        FLinearColor           ColorClearValue  { FLinearColor::Purple() };
    };

    struct FRHIDepthStencilAttachmentDesc
    {
        FRHITextureHandle      Texture;
        ERHIAttachmentLoadOp   DepthLoadOp       { ERHIAttachmentLoadOp::Clear };
        ERHIAttachmentStoreOp  DepthStoreOp      { ERHIAttachmentStoreOp::Store };
        Float                  DepthClearValue   { 1.0f };
        ERHIAttachmentLoadOp   StencilLoadOp     { ERHIAttachmentLoadOp::DontCare };
        ERHIAttachmentStoreOp  StencilStoreOp    { ERHIAttachmentStoreOp::DontCare };
        UInt32                 StencilClearValue { 0 };
    };

    struct FRHIRenderPassAttachments
    {
        TInlineArray<FRHIColorAttachmentDesc, kMaxColorAttachments>
        ColorAttachments;
        FRHIDepthStencilAttachmentDesc
        DepthStencilAttachment;  // Texture = null/invalid means none
    };

    struct VISERA_RUNTIME_API FRHIVertexAttribute
    {
        ERHIFormat Format  { ERHIFormat::Undefined };
        UInt8      Location;
        UInt8      Binding;
        UInt32     Offset;  // Offset in VBO

        Bool operator==(const FRHIVertexAttribute&) const = default;
    };

    struct VISERA_RUNTIME_API FRHIVertexBinding
    {
        ERHIVertexInputRate InputRate {ERHIVertexInputRate::Vertex};
        UInt8               Binding   {0};
        UInt32              Stride    {0};

        Bool operator==(const FRHIVertexBinding&) const = default;
    };
    static_assert(sizeof(FRHIVertexBinding) == 8);

    class VISERA_RUNTIME_API FRHIRenderPassState
    {
    public:
        struct
        {
            TInlineArray<FRHIVertexAttribute, 8> Attributes;
            TInlineArray<FRHIVertexBinding,   8> Bindings;
        }VertexInput; // layout(location = n) in type var

        struct
        {
            ERHIPrimitiveTopology Topology { ERHIPrimitiveTopology::TriangleList };
        }VertexAssembly;

        struct
        {
            ERHIPolygonMode PolygonMode { ERHIPolygonMode::Fill };
            ERHICullMode    CullMode    { ERHICullMode::Back };
            ERHIFrontFace   FrontFace   { ERHIFrontFace::Clockwise };
            Bool            bEnableDepthClamping  = False;
            Bool            bEnableDiscard        = False;
            Bool            bEnableDepthBias      = False;
            struct
            {
                Float DepthBiasSlopeFactor = 1.0f;
                Float LineWidth            = 1.0f;
            }Options;
        }Rasterization;

        struct
        {
            ERHISamplingRate Rate { ERHISamplingRate::X1 };
        }Sampling;

        struct
        {
            ERHIBlendOp Mode { ERHIBlendOp::Add }; // None == Disable
        }ColorBlend;

        struct
        {
            ERHIBlendOp Mode { ERHIBlendOp::Add }; // None == Disable
        }AlphaBlend;

        /** Color attachment formats; length must match render pass ColorAttachments. For MRT (e.g. color RGBA8 + normal R16F). */
        TInlineArray<ERHIFormat, kMaxColorAttachments> ColorFormats;
        /** Depth/stencil format when using a depth attachment; Undefined if no depth. */
        ERHIFormat DepthStencilFormat { ERHIFormat::Undefined };

        Bool operator==(const FRHIRenderPassState& I_Other) const
        {
            return VertexInput.Attributes == I_Other.VertexInput.Attributes &&
                   VertexInput.Bindings == I_Other.VertexInput.Bindings &&
                   VertexAssembly.Topology == I_Other.VertexAssembly.Topology &&
                   Rasterization.PolygonMode == I_Other.Rasterization.PolygonMode &&
                   Rasterization.CullMode == I_Other.Rasterization.CullMode &&
                   Rasterization.FrontFace == I_Other.Rasterization.FrontFace &&
                   Rasterization.bEnableDepthClamping == I_Other.Rasterization.bEnableDepthClamping &&
                   Rasterization.bEnableDiscard == I_Other.Rasterization.bEnableDiscard &&
                   Rasterization.bEnableDepthBias == I_Other.Rasterization.bEnableDepthBias &&
                   Rasterization.Options.DepthBiasSlopeFactor == I_Other.Rasterization.Options.DepthBiasSlopeFactor &&
                   Rasterization.Options.LineWidth == I_Other.Rasterization.Options.LineWidth &&
                   Sampling.Rate == I_Other.Sampling.Rate &&
                   ColorBlend.Mode == I_Other.ColorBlend.Mode &&
                   AlphaBlend.Mode == I_Other.AlphaBlend.Mode &&
                   ColorFormats == I_Other.ColorFormats &&
                   DepthStencilFormat == I_Other.DepthStencilFormat;
        }
    };

    class VISERA_RUNTIME_API FRHIRenderPass : public IRHIRegistryItem
    {
    public:
        struct VISERA_RUNTIME_API FCreateInfo
        {
            FRHIShaderHandle    VertexShader;
            FRHIShaderHandle    FragmentShader;
            FRHIRenderPassState PSO;

            Bool operator==(const FCreateInfo& I_Other) const
            {
                return VertexShader == I_Other.VertexShader &&
                       FragmentShader == I_Other.FragmentShader &&
                       PSO == I_Other.PSO;
            }
            Bool IsCompatibleWith(const FCreateInfo& I_Other) const
            { return *this == I_Other; }
        };

        [[nodiscard]] const FCreateInfo&
        GetInfo() const { return Info; }
        [[nodiscard]] FVulkanRenderPipeline*
        GetVulkanRenderPipeline() { return &Pipeline; }

    private:
        const FCreateInfo     Info;
        FVulkanRenderPipeline Pipeline;

    public:
        FRHIRenderPass() = delete;
        FRHIRenderPass(FCreateInfo&& I_CreateInfo, FVulkanRenderPipeline&& I_Pipeline)
        : Info    {std::move(I_CreateInfo)},
          Pipeline{std::move(I_Pipeline)} {}
    };

    using FRHIRenderPassCreateInfo = FRHIRenderPass::FCreateInfo;
}