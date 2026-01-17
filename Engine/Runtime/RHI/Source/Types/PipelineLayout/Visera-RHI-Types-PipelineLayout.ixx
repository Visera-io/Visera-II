module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Types.PipelineLayout;
#define VISERA_MODULE_NAME "RHI.Types"
import Visera.RHI.Common;
import Visera.RHI.Types.DescriptorSet;
import Visera.Global.Log;
import Visera.Core.Math.Hash.GoldenRatio;
import Visera.Core.Traits.Flags;
import Visera.Core.Types.Array;

export namespace Visera
{


    class VISERA_RHI_API FRHIPipelineLayout
    {
    public:
        inline FRHIPipelineLayout&&
        AddPushConstant(const FRHIPushConstantRange& I_PushConstantRange) && { CachedPipelineLayoutHash.reset(); PushConstantRanges.EmplaceBack(I_PushConstantRange); return std::move(*this); }
        inline FRHIPipelineLayout&&
        AddDescriptorSet(UInt8 I_Index, const FRHIDescriptorSetLayout& I_DescriptorSetLayout) &&;
        inline FRHIPipelineLayout&
        AddPushConstant(const FRHIPushConstantRange& I_PushConstantRange) & { CachedPipelineLayoutHash.reset(); PushConstantRanges.EmplaceBack(I_PushConstantRange); return *this; }
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
        if (I_Index < DescriptorSetLayouts.GetSize())
        { LOG_FATAL("Failed to add the descriptor set ({}) -- already bind!", I_Index); }

        DescriptorSetLayouts.Resize(I_Index + 1); // Must bind by order!
        DescriptorSetLayouts[I_Index] = I_DescriptorSetLayout;
        return std::move(*this);
    }

    inline FRHIPipelineLayout& FRHIPipelineLayout::
    AddDescriptorSet(UInt8 I_Index, const FRHIDescriptorSetLayout& I_DescriptorSetLayout) &
    {
        CachedPipelineLayoutHash.reset();
        if (I_Index < DescriptorSetLayouts.GetSize())
        { LOG_FATAL("Failed to add the descriptor set ({}) -- already bind!", I_Index); }

        DescriptorSetLayouts.Resize(I_Index + 1); // Must bind by order!
        DescriptorSetLayouts[I_Index] = I_DescriptorSetLayout;
        return *this;
    }
}