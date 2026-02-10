module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.Resource.RenderPass;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.Containers.Array;
import Visera.Runtime.RHI.Common;
import Visera.Runtime.RHI.Registry.Item;
import Visera.Runtime.RHI.Vulkan.Pipeline.Render;
export namespace Visera
{
    struct VISERA_RUNTIME_API FRHIVertexAttribute
    {
        ERHIFormat Format  { ERHIFormat::Undefined };
        UInt8      Location;
        UInt8      Binding;
        UInt32     Offset;  // Offset in VBO
    };

    struct VISERA_RUNTIME_API FRHIVertexBinding
    {
        ERHIVertexInputRate InputRate {ERHIVertexInputRate::Vertex};
        UInt8               Binding   {0};
        UInt32              Stride    {0};
    };
    static_assert(sizeof(FRHIVertexBinding) == 8);

    class VISERA_RUNTIME_API FRHIRenderPassDesc
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

        Bool operator==(const FRHIRenderPassDesc&) const = default;
    };

    class VISERA_RUNTIME_API FRHIRenderPass : public IRHIRegistryItem
    {
    public:
        using FCreateInfo = FRHIRenderPassDesc;

        [[nodiscard]] const FCreateInfo&
        GetInfo() const { return Info; }
        [[nodiscard]] FVulkanRenderPipeline*
        GetVulkanRenderPipeline() { return &Pipeline; }

    private:
        FCreateInfo           Info;
        FVulkanRenderPipeline Pipeline;

    public:
        FRHIRenderPass() = delete;
        FRHIRenderPass(FCreateInfo&& I_Desc, FVulkanRenderPipeline&& I_Pipeline)
        : Info    {std::move(I_Desc)},
          Pipeline{std::move(I_Pipeline)} {}
    };
}