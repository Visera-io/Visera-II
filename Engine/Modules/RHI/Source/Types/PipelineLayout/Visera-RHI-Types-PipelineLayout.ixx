module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Types.PipelineLayout;
#define VISERA_MODULE_NAME "RHI.Types"
import Visera.RHI.Common;
import Visera.RHI.Types.DescriptorSet;
import Visera.Runtime.Log;
import Visera.Core.Math.Hash.GoldenRatio;
import Visera.Core.Traits.Flags;
import Visera.Core.Types.Array;

export namespace Visera
{
    struct VISERA_RHI_API FRHIPushConstantRange
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

    class VISERA_RHI_API FRHIPipelineLayout
    {
    public:
        inline FRHIPipelineLayout&&
        AddPushConstant(const FRHIPushConstantRange& I_PushConstantRange) && { CachedPipelineLayoutHash.reset(); PushConstantRanges.emplace_back(I_PushConstantRange); return std::move(*this); }
        inline FRHIPipelineLayout&&
        AddDescriptorSet(UInt8 I_Index, const FRHIDescriptorSetLayout& I_DescriptorSetLayout) &&;
        inline FRHIPipelineLayout&
        AddPushConstant(const FRHIPushConstantRange& I_PushConstantRange) & { CachedPipelineLayoutHash.reset(); PushConstantRanges.emplace_back(I_PushConstantRange); return *this; }
        inline FRHIPipelineLayout&
        AddDescriptorSet(UInt8 I_Index, const FRHIDescriptorSetLayout& I_DescriptorSetLayout) &;
        [[nodiscard]] inline const TArray<FRHIPushConstantRange>&
        GetPushConstantRanges() const { return PushConstantRanges; }
        [[nodiscard]] inline const TArray<FRHIDescriptorSetLayout>&
        GetDescriptorLayouts() const { return DescriptorSetLayouts; }

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


    inline FRHIPipelineLayout&& FRHIPipelineLayout::
    AddDescriptorSet(UInt8 I_Index, const FRHIDescriptorSetLayout& I_DescriptorSetLayout) &&
    {
        CachedPipelineLayoutHash.reset();
        if (I_Index < DescriptorSetLayouts.size())
        { LOG_FATAL("Failed to add the descriptor set ({}) -- already bind!", I_Index); }

        DescriptorSetLayouts.resize(I_Index + 1); // Must bind by order!
        DescriptorSetLayouts[I_Index] = I_DescriptorSetLayout;
        return std::move(*this);
    }

    inline FRHIPipelineLayout& FRHIPipelineLayout::
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