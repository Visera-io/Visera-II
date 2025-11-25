module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Types.DescriptorSet;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Common;
import Visera.Core.Hash.GoldenRatio;
import Visera.Core.Traits.Flags;
import Visera.Core.Log;

export namespace Visera
{
    struct VISERA_RUNTIME_API FRHISetBinding
    {
        UInt32              Index        {0};
        ERHISetBindingType  Type         {ERHISetBindingType::Undefined};
        UInt32              Count        {0};
        ERHIShaderStages    ShaderStages {ERHIShaderStages::Undefined};

        UInt64              ImmutableSamplerID {0 /*None = 0*/};

        // ---- Factories ----
        [[nodiscard]] static constexpr FRHISetBinding
        UniformBuffer(UInt32 I_Index, ERHIShaderStages I_Stages, UInt32 I_Count = 1) noexcept
        { return { I_Index, ERHISetBindingType::UniformBuffer, I_Count, I_Stages }; }

        [[nodiscard]] static constexpr FRHISetBinding
        StorageBuffer(UInt32 I_Index, ERHIShaderStages I_Stages, UInt32 I_Count = 1) noexcept
        { return { I_Index, ERHISetBindingType::StorageBuffer, I_Count, I_Stages }; }

        [[nodiscard]] static constexpr FRHISetBinding
        CombinedImageSampler(UInt32 I_Index, ERHIShaderStages I_Stages, UInt32 I_Count = 1, UInt64 immutableSamplerID = 0) noexcept
        { return { I_Index, ERHISetBindingType::CombinedImageSampler, I_Count, I_Stages, immutableSamplerID }; }

        [[nodiscard]] constexpr Bool
        IsImmutableSampler() const noexcept
        { return (Type == ERHISetBindingType::CombinedImageSampler) && ImmutableSamplerID != 0; }

        [[nodiscard]] constexpr Bool
        IsValid() const noexcept
        {
            return Type         != ERHISetBindingType::Undefined &&
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

    struct VISERA_RUNTIME_API FRHISetLayout
    {
        TArray<FRHISetBinding> Bindings;

        [[nodiscard]] UInt64
        Hash(UInt64 I_Seed) const noexcept
        {
            UInt64 Seed = 0;
            for (const auto& Binding : Bindings)
            {
                Seed = GoldenRatioHash(Seed, Binding.Hash());
            }
            return Seed;
        }
    };
}