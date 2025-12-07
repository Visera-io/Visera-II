module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Types.DescriptorSet;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Common;
import Visera.Runtime.RHI.Vulkan.DescriptorSet;
import Visera.Core.Math.Hash.GoldenRatio;
import Visera.Core.Traits.Flags;
import Visera.Core.Log;

export namespace Visera
{
    using FRHIDescriptorSet = FVulkanDescriptorSet;

    struct VISERA_RUNTIME_API FRHIDescriptorSetBinding
    {
        UInt32              Binding      {0};
        ERHIDescriptorType  Type         {ERHIDescriptorType::Undefined};
        UInt32              Count        {0};
        ERHIShaderStages    ShaderStages {ERHIShaderStages::Undefined};

        UInt64              ImmutableSamplerID {0 /*None = 0*/};

        // ---- Factories ----
        [[nodiscard]] static constexpr FRHIDescriptorSetBinding
        UniformBuffer(UInt32 I_Binding, ERHIShaderStages I_Stages, UInt32 I_Count = 1) noexcept
        { return { I_Binding, ERHIDescriptorType::UniformBuffer, I_Count, I_Stages }; }

        [[nodiscard]] static constexpr FRHIDescriptorSetBinding
        StorageBuffer(UInt32 I_Binding, ERHIShaderStages I_Stages, UInt32 I_Count = 1) noexcept
        { return { I_Binding, ERHIDescriptorType::StorageBuffer, I_Count, I_Stages }; }

        [[nodiscard]] static constexpr FRHIDescriptorSetBinding
        CombinedImageSampler(UInt32 I_Binding, ERHIShaderStages I_Stages, UInt32 I_Count = 1, UInt64 immutableSamplerID = 0) noexcept
        { return { I_Binding, ERHIDescriptorType::CombinedImageSampler, I_Count, I_Stages, immutableSamplerID }; }

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
        operator==(const FRHIDescriptorSetBinding& I_A, const FRHIDescriptorSetBinding& I_B) noexcept = default;

        friend constexpr auto operator<=>(const FRHIDescriptorSetBinding& a,
                                          const FRHIDescriptorSetBinding& b) noexcept
        {
            if (a.Binding != b.Binding) { return a.Binding <=> b.Binding; }
            if (a.Type  != b.Type)  { return a.Type  <=> b.Type;  }
            if (a.Count != b.Count) { return a.Count <=> b.Count; }
            if (ToUnderlying(a.ShaderStages) != ToUnderlying(b.ShaderStages))
            { return ToUnderlying(a.ShaderStages) <=> ToUnderlying(b.ShaderStages); }

            return a.ImmutableSamplerID <=> b.ImmutableSamplerID;
        }

        [[nodiscard]] UInt64
        Hash(UInt64 I_Seed = 0) const noexcept
        {
            return GoldenRatioHashCombine(I_Seed,
                Binding,
                Type,
                Count,
                ShaderStages,
                ImmutableSamplerID);
        }
    };

    class VISERA_RUNTIME_API FRHIDescriptorSetLayout
    {
    public:
        [[nodiscard]] inline const TArray<FRHIDescriptorSetBinding>&
        GetBindings() const { return Bindings; }
        inline FRHIDescriptorSetLayout&&
        AddBinding(UInt32               I_Binding,
                   ERHIDescriptorType   I_Type,
                   UInt32               I_Count,
                   ERHIShaderStages     I_Stages,
                   UInt64               ImmutableSamplerID = 0) && noexcept
        { CachedHash.reset(); Bindings.emplace_back(I_Binding, I_Type, I_Count, I_Stages, ImmutableSamplerID); return std::move(*this); }
        inline FRHIDescriptorSetLayout&
        AddBinding(UInt32               I_Binding,
                   ERHIDescriptorType   I_Type,
                   UInt32               I_Count,
                   ERHIShaderStages     I_Stages,
                   UInt64               ImmutableSamplerID = 0) & noexcept
        { CachedHash.reset(); Bindings.emplace_back(I_Binding, I_Type, I_Count, I_Stages, ImmutableSamplerID); return *this; }
        inline FRHIDescriptorSetLayout&&
        AddBinding(const FRHIDescriptorSetBinding& I_Prefab) && noexcept
        { CachedHash.reset(); Bindings.emplace_back(I_Prefab); return std::move(*this); }
        inline FRHIDescriptorSetLayout&
        AddBinding(const FRHIDescriptorSetBinding& I_Prefab) & noexcept
        { CachedHash.reset(); Bindings.emplace_back(I_Prefab); return *this; }

        [[nodiscard]] UInt64
        Hash() const noexcept
        {
            if (CachedHash.has_value())
            { return CachedHash.value(); }

            UInt64 Seed = 0;
            for (const auto& Binding : Bindings)
            {
                Seed = GoldenRatioHash(Seed, Binding.Hash());
            }
            CachedHash = Seed;
            return CachedHash.value();
        }
    private:
        mutable TArray<FRHIDescriptorSetBinding> Bindings;
        mutable TOptional<UInt64>      CachedHash;
    };
}