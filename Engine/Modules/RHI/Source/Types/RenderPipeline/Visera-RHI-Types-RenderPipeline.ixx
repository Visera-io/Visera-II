module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Types.RenderPipeline;
#define VISERA_MODULE_NAME "RHI.Types"
import Visera.RHI.Vulkan.Pipeline.Render;
import Visera.RHI.Types.PipelineLayout;
import Visera.RHI.Types.Shader;
import Visera.RHI.Common;
import Visera.Runtime.Log;

export namespace Visera
{
    using FRHIRenderPipeline = FVulkanRenderPipeline;


    struct VISERA_RHI_API FRHIVertexAttribute
    {
        UInt8      Location;
        UInt8      Binding; // From which VBO
        ERHIFormat Format  { ERHIFormat::Undefined };
        UInt32     Offset;  // Offset in VBO
    };

    struct VISERA_RHI_API FRHIVertexBinding
    {
        UInt8  Binding {0};         // From which VBO
        Bool   bByInstance = False; // If True the "pointer + stride" happens when drawInstance
        UInt32 Stride  {0};
    };

    class VISERA_RHI_API FRHIRenderPipelineDesc
    {
    public:
        FRHIPipelineLayout              Layout;

        TSharedPtr<FRHIShader>          VertexShader;
        TSharedPtr<FRHIShader>          FragmentShader;

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
            Bool            bClockwiseIsFrontFace = True;
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

}