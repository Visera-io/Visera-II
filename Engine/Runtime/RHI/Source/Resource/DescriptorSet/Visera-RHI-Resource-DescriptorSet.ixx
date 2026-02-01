module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Resource.DescriptorSet;
#define VISERA_MODULE_NAME "RHI.Resource"
import Visera.RHI.Common;
import Visera.RHI.Vulkan.DescriptorSet;
import Visera.Core.Types.Array;
import vulkan_hpp;

export namespace Visera
{
    struct VISERA_RHI_API FRHIDescriptorSetCreateDesc
    {
        TArray<vk::DescriptorSetLayoutBinding> Bindings;

        Bool IsCompatibleWith(const FRHIDescriptorSetCreateDesc& I_Other) const
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

    class VISERA_RHI_API FRHIDescriptorSet
    {
    public:
        [[nodiscard]] const FRHIDescriptorSetCreateDesc&
        GetInfo() const { return Info; }
        [[nodiscard]] FVulkanDescriptorSet*
        GetVulkanDescriptorSet() { return &DescriptorSet; }
        [[nodiscard]] const FVulkanDescriptorSet*
        GetVulkanDescriptorSet() const { return &DescriptorSet; }

    private:
        FRHIDescriptorSetCreateDesc Info;
        FVulkanDescriptorSet        DescriptorSet;

    public:
        FRHIDescriptorSet() = delete;
        FRHIDescriptorSet(FRHIDescriptorSetCreateDesc&& I_CreateDesc, FVulkanDescriptorSet&& I_VulkanDescriptorSet)
        : Info         {std::move(I_CreateDesc)},
          DescriptorSet{std::move(I_VulkanDescriptorSet)} {}
    };
}
