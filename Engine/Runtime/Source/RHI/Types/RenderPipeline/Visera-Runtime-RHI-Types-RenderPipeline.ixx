module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Types.RenderPipeline;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Common;
import Visera.Runtime.RHI.Types.Shader;
import Visera.Runtime.RHI.Types.DescriptorSet;
import Visera.Runtime.RHI.Vulkan.Pipeline.Render;
import Visera.Core.Math.Hash.GoldenRatio;
import Visera.Core.Traits.Flags;
import Visera.Core.Types.Array;
import Visera.Core.Log;

export namespace Visera
{
    using FRHIRenderPipeline = FVulkanRenderPipeline;

    struct VISERA_RUNTIME_API FRHIPushConstantRange
    {
        UInt32           Size   {0};
        UInt32           Offset {0};
        ERHIShaderStages Stages {ERHIShaderStages::Undefined};

        [[nodiscard]] inline UInt64
        Hash() const { return Math::GoldenRatioHashCombine(0, Size, Offset, Stages); }

        friend constexpr Bool
        operator==(const FRHIPushConstantRange& I_A, const FRHIPushConstantRange& I_B) noexcept = default;

        friend constexpr auto operator<=>(const FRHIPushConstantRange& a,
                                          const FRHIPushConstantRange& b) noexcept
        {
            if (a.Size   != b.Size)   { return a.Size   <=> b.Size;   }
            if (a.Offset != b.Offset) { return a.Offset <=> b.Offset; }
            return ToUnderlying(a.Stages) <=> ToUnderlying(b.Stages);
        }
    };

    struct VISERA_RUNTIME_API FRHIVertexAttribute
    {
        UInt8      Location;
        UInt8      Binding; // From which VBO
        ERHIFormat Format  { ERHIFormat::Undefined };
        UInt32     Offset;  // Offset in VBO
    };

    struct VISERA_RUNTIME_API FRHIVertexBinding
    {
        UInt8  Binding {0};         // From which VBO
        Bool   bByInstance = False; // If True the "pointer + stride" happens when drawInstance
        UInt32 Stride  {0};
    };

    class VISERA_RUNTIME_API FRHIRenderPipelineState
    {
    public:
        inline FRHIRenderPipelineState&&
        AddPushConstant(const FRHIPushConstantRange& I_PushConstantRange) && { CachedPipelineLayoutHash.reset(); PushConstantRanges.emplace_back(I_PushConstantRange); return std::move(*this); }
        inline FRHIRenderPipelineState&&
        AddDescriptorSet(UInt8 I_Index, const FRHIDescriptorSetLayout& I_DescriptorSetLayout) &&;
        inline FRHIRenderPipelineState&
        AddPushConstant(const FRHIPushConstantRange& I_PushConstantRange) & { CachedPipelineLayoutHash.reset(); PushConstantRanges.emplace_back(I_PushConstantRange); return *this; }
        inline FRHIRenderPipelineState&
        AddDescriptorSet(UInt8 I_Index, const FRHIDescriptorSetLayout& I_DescriptorSetLayout) &;
        [[nodiscard]] inline const TArray<FRHIPushConstantRange>&
        GetPushConstantRanges() const { return PushConstantRanges; }
        [[nodiscard]] inline const TArray<FRHIDescriptorSetLayout>&
        GetDescriptorLayouts() const { return DescriptorSetLayouts; }

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

        [[nodiscard]] inline UInt64
        GetPipelineLayoutHash() const
        {
            if (CachedPipelineLayoutHash.has_value())
            { return CachedPipelineLayoutHash.value(); }

            std::ranges::sort(PushConstantRanges);

            UInt64 Seed = 0;
            {
                for (const auto& PushConstant : PushConstantRanges)
                { Seed = Math::GoldenRatioHash(Seed, PushConstant.Hash()); }
                for (const auto& DescriptorSet : DescriptorSetLayouts)
                { Seed = Math::GoldenRatioHash(Seed, DescriptorSet.Hash()); }
            }
            CachedPipelineLayoutHash = Seed;
            return CachedPipelineLayoutHash.value();
        }

    private:
        mutable TArray<FRHIPushConstantRange>   PushConstantRanges;
        mutable TArray<FRHIDescriptorSetLayout> DescriptorSetLayouts; // DescriptorSetLayouts[i] == set(i)
        mutable TOptional<UInt64>               CachedPipelineLayoutHash;
    };

    inline FRHIRenderPipelineState&& FRHIRenderPipelineState::
    AddDescriptorSet(UInt8 I_Index, const FRHIDescriptorSetLayout& I_DescriptorSetLayout) &&
    {
        CachedPipelineLayoutHash.reset();
        if (I_Index < DescriptorSetLayouts.size())
        { LOG_FATAL("Failed to add the descriptor set ({}) -- already bind!", I_Index); }

        DescriptorSetLayouts.resize(I_Index + 1); // Must bind by order!
        DescriptorSetLayouts[I_Index] = I_DescriptorSetLayout;
        return std::move(*this);
    }

    inline FRHIRenderPipelineState& FRHIRenderPipelineState::
    AddDescriptorSet(UInt8 I_Index, const FRHIDescriptorSetLayout& I_DescriptorSetLayout) &
    {
        CachedPipelineLayoutHash.reset();
        if (I_Index < DescriptorSetLayouts.size())
        { LOG_FATAL("Failed to add the descriptor set ({}) -- already bind!", I_Index); }

        DescriptorSetLayouts.resize(I_Index + 1); // Must bind by order!
        DescriptorSetLayouts[I_Index] = I_DescriptorSetLayout;
        return *this;
    }
}