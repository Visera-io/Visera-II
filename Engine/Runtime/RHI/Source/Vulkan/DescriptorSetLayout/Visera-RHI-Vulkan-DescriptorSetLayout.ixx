module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Vulkan.DescriptorSetLayout;
#define VISERA_MODULE_NAME "RHI.Vulkan"
import Visera.RHI.Vulkan.Common;
import Visera.Global.Log;
import Visera.Core.Types.Array;
import vulkan_hpp;

namespace Visera
{
    export class VISERA_RHI_API FVulkanDescriptorSetLayout
    {
    public:
        [[nodiscard]] inline const vk::raii::DescriptorSetLayout&
        GetHandle() const { return Handle; }

    private:
        vk::raii::DescriptorSetLayout          Handle {nullptr};

    public:
        FVulkanDescriptorSetLayout() = default;
        FVulkanDescriptorSetLayout(const vk::raii::Device&                       I_Device,
                                   const TArray<vk::DescriptorSetLayoutBinding>& I_Bindings);
        FVulkanDescriptorSetLayout(const vk::raii::Device&                       I_Device,
                                   const TArray<vk::DescriptorSetLayoutBinding>& I_Bindings,
                                   const TArray<vk::DescriptorBindingFlags>&     I_BindingFlags);
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