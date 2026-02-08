module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Resource.RenderPass;
#define VISERA_MODULE_NAME "RHI.Resource"
import Visera.Core.Types.Array;
import Visera.RHI.Common;
import Visera.RHI.Vulkan.Pipeline.Render;
export namespace Visera
{
    struct VISERA_RHI_API FRHIVertexAttribute
    {
        ERHIFormat Format  { ERHIFormat::Undefined };
        UInt8      Location;
        UInt8      Binding;
        UInt32     Offset;  // Offset in VBO
    };

    struct VISERA_RHI_API FRHIVertexBinding
    {
        ERHIVertexInputRate InputRate {ERHIVertexInputRate::Vertex};
        UInt8               Binding   {0};
        UInt32              Stride    {0};
    };
    static_assert(sizeof(FRHIVertexBinding) == 8);

    class VISERA_RHI_API FRHIRenderPassDesc
    {
    public:
        struct
        {

        }VertexShader;

        struct
        {

        }FragmentShader;

        struct
        {
            TArray<FRHIVertexAttribute> Attributes;
            TArray<FRHIVertexBinding>   Bindings;
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
    };

    class VISERA_RHI_API FRHIRenderPass
    {
    public:
        [[nodiscard]] const FRHIRenderPassDesc&
        GetInfo() const { return Info; }
        [[nodiscard]] FVulkanRenderPipeline*
        GetVulkanRenderPipeline() { return &Pipeline; }

    private:
        FRHIRenderPassDesc    Info;
        FVulkanRenderPipeline Pipeline;

    public:
        FRHIRenderPass() = delete;
        FRHIRenderPass(FRHIRenderPassDesc&& I_Desc, FVulkanRenderPipeline&& I_Pipeline)
        : Info    {std::move(I_Desc)},
          Pipeline{std::move(I_Pipeline)} {}
    };
}