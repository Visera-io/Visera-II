module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Types.DescriptorSet;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Common;
import Visera.Runtime.RHI.Vulkan.DescriptorSet;
import Visera.Core.Hash.GoldenRatio;
import Visera.Core.Traits.Flags;
import Visera.Core.Log;

export namespace Visera
{
    using FRHIDescriptorSet = FVulkanDescriptorSet;

    struct VISERA_RUNTIME_API FRHISetBinding
    {
        UInt32              Index        {0};
        ERHIDescriptorType  Type         {ERHIDescriptorType::Undefined};
        UInt32              Count        {0};
        ERHIShaderStages    ShaderStages {ERHIShaderStages::Undefined};

        UInt64              ImmutableSamplerID {0 /*None = 0*/};

        // ---- Factories ----
        [[nodiscard]] static constexpr FRHISetBinding
        UniformBuffer(UInt32 I_Index, ERHIShaderStages I_Stages, UInt32 I_Count = 1) noexcept
        { return { I_Index, ERHIDescriptorType::UniformBuffer, I_Count, I_Stages }; }

        [[nodiscard]] static constexpr FRHISetBinding
        StorageBuffer(UInt32 I_Index, ERHIShaderStages I_Stages, UInt32 I_Count = 1) noexcept
        { return { I_Index, ERHIDescriptorType::StorageBuffer, I_Count, I_Stages }; }

        [[nodiscard]] static constexpr FRHISetBinding
        CombinedImageSampler(UInt32 I_Index, ERHIShaderStages I_Stages, UInt32 I_Count = 1, UInt64 immutableSamplerID = 0) noexcept
        { return { I_Index, ERHIDescriptorType::CombinedImageSampler, I_Count, I_Stages, immutableSamplerID }; }

        [[nodiscard]] constexpr Bool
        IsImmutableSampler() const noexcept
        { return (Type == ERHIDescriptorType::CombinedImageSampler) && ImmutableSamplerID != 0; }

        [[nodiscard]] constexpr Bool
        IsValid() const noexcept
        {
            return Type         != ERHIDescriptorType::Undefined &&
                   Count        != 0                             &&
                   ShaderStages != ERHIShaderStages::Undefined;
        }

        friend constexpr Bool
        operator==(const FRHISetBinding& I_A, const FRHISetBinding& I_B) noexcept = default;

        friend constexpr auto operator<=>(const FRHISetBinding& a,
                                          const FRHISetBinding& b) noexcept
        {
            if (a.Index != b.Index) return a.Index <=> b.Index;
            if (a.Type  != b.Type)  return a.Type  <=> b.Type;
            if (a.Count != b.Count) return a.Count <=> b.Count;
            if (ToUnderlying(a.ShaderStages) != ToUnderlying(b.ShaderStages))
                return ToUnderlying(a.ShaderStages) <=> ToUnderlying(b.ShaderStages);
            return a.ImmutableSamplerID <=> b.ImmutableSamplerID;
        }

        [[nodiscard]] UInt64
        Hash(UInt64 I_Seed = 0) const noexcept
        {
            return GoldenRatioHashCombine(I_Seed,
                Index,
                Type,
                Count,
                ShaderStages,
                ImmutableSamplerID);
        }
    };

    class VISERA_RUNTIME_API FRHIDescriptorSetLayout
    {
    public:
        [[nodiscard]] inline UInt32
        GetBindingCount() const { return Bindings.size(); }
        [[nodiscard]] inline const FRHISetBinding&
        GetBinding(UInt32 I_Index) const { VISERA_ASSERT(I_Index < GetBindingCount()); return Bindings[I_Index]; }
        inline auto&&
        AddBinding(UInt32               I_Index,
                   ERHIDescriptorType   I_Type,
                   UInt32               I_Count,
                   ERHIShaderStages     I_Stages,
                   UInt64               ImmutableSamplerID = 0) noexcept
        { bSorted = False; Bindings.emplace_back(I_Index, I_Type, I_Count, I_Stages, ImmutableSamplerID); return std::move(*this); }

        [[nodiscard]] UInt64
        GetOrderedHash() const noexcept
        {
            if (!bSorted)
            {
                std::ranges::sort(Bindings.begin(), Bindings.end());
                bSorted = True;
            }
            UInt64 Seed = 0;
            for (const auto& Binding : Bindings)
            {
                Seed = GoldenRatioHash(Seed, Binding.Hash());
            }
            return Seed;
        }
    private:
        mutable TArray<FRHISetBinding> Bindings;
        mutable Bool                   bSorted = False;
    };
}