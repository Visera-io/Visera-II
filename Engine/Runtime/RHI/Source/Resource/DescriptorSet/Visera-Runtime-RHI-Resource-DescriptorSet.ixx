module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.Resource.DescriptorSet;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Common;
import Visera.Runtime.RHI.Registry.Item;
import Visera.Runtime.RHI.Vulkan.DescriptorSet;
import Visera.Core.Types.Array;

export namespace Visera
{
    struct VISERA_RUNTIME_API FRHIDescriptorSetLayoutBinding
    {
        UInt8 binding;
        ERHIDescriptorType descriptorType;
        UInt32 descriptorCount;
        ERHIShaderStage stageFlags;

        Bool operator==(const FRHIDescriptorSetLayoutBinding&) const = default;
    };

    class VISERA_RUNTIME_API FRHIDescriptorSet : public IRHIRegistryItem
    {
    public:
        struct VISERA_RUNTIME_API FCreateInfo
        {
            TArray<FRHIDescriptorSetLayoutBinding> Bindings;

            Bool operator==(const FCreateInfo&) const = default;
            Bool IsCompatibleWith(const FCreateInfo& I_Other) const
            {
                if (Bindings.GetSize() != I_Other.Bindings.GetSize()) { return False; }
                for (UInt32 Idx = 0; Idx < Bindings.GetSize(); ++Idx)
                {
                    const auto& A = Bindings[Idx];
                    const auto& B = I_Other.Bindings[Idx];
                    if (A.binding != B.binding ||
                        A.descriptorType != B.descriptorType ||
                        A.descriptorCount != B.descriptorCount ||
                        A.stageFlags != B.stageFlags)
                    { return False; }
                }
                return True;
            }
        };

        [[nodiscard]] const FCreateInfo&
        GetInfo() const { return Info; }
        [[nodiscard]] FVulkanDescriptorSet*
        GetVulkanDescriptorSet() { return &DescriptorSet; }
        [[nodiscard]] const FVulkanDescriptorSet*
        GetVulkanDescriptorSet() const { return &DescriptorSet; }

    private:
        const FCreateInfo Info;
        FVulkanDescriptorSet DescriptorSet;

    public:
        FRHIDescriptorSet() = delete;
        FRHIDescriptorSet(FCreateInfo&& I_CreateInfo, FVulkanDescriptorSet&& I_VulkanDescriptorSet)
        : Info         {std::move(I_CreateInfo)},
          DescriptorSet{std::move(I_VulkanDescriptorSet)} {}
    };

    using FRHIDescriptorSetCreateInfo = FRHIDescriptorSet::FCreateInfo;
}
