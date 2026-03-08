module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.Vulkan.DescriptorPool;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.DescriptorSet;
import Visera.Runtime.RHI.Vulkan.DescriptorSetLayout;
import Visera.Core.Containers.Array;
import Visera.Core.Log;
import vulkan_hpp;

export namespace Visera
{
    /**
     * Vulkan descriptor pool. PoolSizes and MaxSets must be chosen so that every
     * FVulkanDescriptorSetLayout that allocates from this pool has its bindings
     * covered: for each descriptor type, the sum of descriptorCount across
     * bindings of that type must not exceed the pool's corresponding
     * VkDescriptorPoolSize::descriptorCount, and total sets must not exceed MaxSets.
     */
    class VISERA_RUNTIME_API FVulkanDescriptorPool
    {
    public:
        [[nodiscard]] FVulkanDescriptorSet
        CreateDescriptorSet(const FVulkanDescriptorSetLayout& I_DescriptorSetLayout);
        [[nodiscard]] const vk::raii::DescriptorPool&
        GetHandle() const { return Handle; }

    private:
        vk::raii::DescriptorPool Handle {nullptr};
        UInt32                   AvailableSets{0};

    public:
        FVulkanDescriptorPool() = default;
        FVulkanDescriptorPool(const vk::raii::Device&               I_Device,
                              const TArray<vk::DescriptorPoolSize>& I_PoolSizes,
                              UInt32                                I_MaxSets);
    };

    FVulkanDescriptorPool::
    FVulkanDescriptorPool(const vk::raii::Device&               I_Device,
                          const TArray<vk::DescriptorPoolSize>& I_PoolSizes,
                          UInt32                                I_MaxSets)
    : AvailableSets{I_MaxSets}
    {
        const auto CreateInfo = vk::DescriptorPoolCreateInfo{}
            .setFlags           (vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .setPoolSizeCount   (I_PoolSizes.GetSize())
            .setPPoolSizes      (I_PoolSizes.Data())
            .setMaxSets         (I_MaxSets)
        ;
        auto Result = I_Device.createDescriptorPool(CreateInfo);
        if (!Result.has_value())
        { LOG_FATAL("Failed to create Vulkan DescriptorPool!"); }
        else
        { Handle = std::move(*Result); }
    }

    FVulkanDescriptorSet FVulkanDescriptorPool::
    CreateDescriptorSet(const FVulkanDescriptorSetLayout& I_DescriptorSetLayout)
    {
        VISERA_ASSERT(AvailableSets > 0);

        AvailableSets -= 1;

        auto Layout = I_DescriptorSetLayout.GetHandle();
        auto CreateInfo = vk::DescriptorSetAllocateInfo{}
            .setDescriptorPool      (Handle)
            .setDescriptorSetCount  (1)
            .setPSetLayouts         (&Layout)
        ;
        return FVulkanDescriptorSet(Handle, CreateInfo);
    }
}
