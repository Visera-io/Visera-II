module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Resource.RenderPass;
#define VISERA_MODULE_NAME "RHI.Resource"
import Visera.RHI.Common;
import Visera.RHI.Vulkan.Pipeline.Render;
import Visera.Core.Types.Array;

export namespace Visera
{
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

    struct VISERA_RHI_API FRHIDescriptorSetBinding
    {
        UInt32              Binding      {0};
        ERHIDescriptorType  Type         {ERHIDescriptorType::Undefined};
        UInt32              Count        {0};
        ERHIShaderStages    ShaderStages {ERHIShaderStages::Undefined};

        UInt64              ImmutableSamplerID {0 /*None = 0*/};
    };

    struct VISERA_RHI_API FRHIPushConstantRange
    {
        UInt32           Size   {0};
        UInt32           Offset {0};
        ERHIShaderStages Stages {ERHIShaderStages::Undefined};

        friend constexpr Bool
        operator==(const FRHIPushConstantRange& I_A, const FRHIPushConstantRange& I_B) noexcept = default;

        friend constexpr auto
        operator<=>(const FRHIPushConstantRange& a, const FRHIPushConstantRange& b) noexcept
        {
            if (a.Size   != b.Size)   { return a.Size   <=> b.Size;   }
            if (a.Offset != b.Offset) { return a.Offset <=> b.Offset; }
            return ToUnderlying(a.Stages) <=> ToUnderlying(b.Stages);
        }
    };

    class VISERA_RHI_API FRHIRenderPassDesc
    {
    public:
        TArray<FRHIPushConstantRange>   PushConstantRanges;
        TArray<FRHIDescriptorSetBinding>DescriptorSetBindings;

        TArray<FByte>                   VertexShader;
        TArray<FByte>                   FragmentShader;

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

    class VISERA_RHI_API FRHIRenderPass
    {
    public:


    private:
        FVulkanRenderPipeline Pipeline;
    };
}