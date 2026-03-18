module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Vulkan.DescriptorSetLayout;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Core.Log;
import Visera.Core.Containers.Array;
import vulkan_hpp;

export namespace Visera
{
    class VISERA_RUNTIME_API FVulkanDescriptorSetLayout
    {
    public:
        [[nodiscard]] inline vk::DescriptorSetLayout
        GetHandle() const { return Handle; }

    private:
        vk::raii::DescriptorSetLayout Handle {nullptr};

    public:
        FVulkanDescriptorSetLayout() = default;
        FVulkanDescriptorSetLayout(const vk::raii::Device&                       I_Device,
                                   const TArray<vk::DescriptorSetLayoutBinding>& I_Bindings);
        FVulkanDescriptorSetLayout(const vk::raii::Device&                       I_Device,
                                   const TArray<vk::DescriptorSetLayoutBinding>& I_Bindings,
                                   const TArray<vk::DescriptorBindingFlags>&     I_BindingFlags);
        FVulkanDescriptorSetLayout(const FVulkanDescriptorSetLayout&)            = delete;
        FVulkanDescriptorSetLayout& operator=(const FVulkanDescriptorSetLayout&) = delete;
        FVulkanDescriptorSetLayout(FVulkanDescriptorSetLayout&&)                 = default;
        FVulkanDescriptorSetLayout& operator=(FVulkanDescriptorSetLayout&&)      = default;
    };

    FVulkanDescriptorSetLayout::
    FVulkanDescriptorSetLayout(const vk::raii::Device&                       I_Device,
                               const TArray<vk::DescriptorSetLayoutBinding>& I_Bindings)
    {
        auto CreateInfo = vk::DescriptorSetLayoutCreateInfo{}
            .setBindingCount    (I_Bindings.GetSize())
            .setPBindings       (I_Bindings.Data())
        ;
        auto Result = I_Device.createDescriptorSetLayout(CreateInfo);
        if (Result.has_value())
        { Handle = std::move(*Result); }
        else
        { LOG_FATAL("Failed to create descriptor set layout!"); }
    }

    FVulkanDescriptorSetLayout::
    FVulkanDescriptorSetLayout(const vk::raii::Device&                       I_Device,
                               const TArray<vk::DescriptorSetLayoutBinding>& I_Bindings,
                               const TArray<vk::DescriptorBindingFlags>&     I_BindingFlags)
    {
        VISERA_ASSERT(I_Bindings.GetSize() == I_BindingFlags.GetSize());
        
        auto BindingFlagsInfo = vk::DescriptorSetLayoutBindingFlagsCreateInfo{}
            .setBindingCount    (I_BindingFlags.GetSize())
            .setPBindingFlags   (I_BindingFlags.Data())
        ;

        auto CreateInfo = vk::DescriptorSetLayoutCreateInfo{}
            .setPNext           (&BindingFlagsInfo)
            .setFlags           (vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool)
            .setBindingCount    (I_Bindings.GetSize())
            .setPBindings       (I_Bindings.Data())
        ;
        auto Result = I_Device.createDescriptorSetLayout(CreateInfo);
        if (Result.has_value())
        { Handle = std::move(*Result); }
        else
        { LOG_FATAL("Failed to create descriptor set layout!"); }
    }
}