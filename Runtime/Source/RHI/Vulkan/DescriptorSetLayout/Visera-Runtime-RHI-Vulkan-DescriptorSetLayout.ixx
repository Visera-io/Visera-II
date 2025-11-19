module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.DescriptorSetLayout;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanDescriptorSetLayout
    {
    public:
        [[nodiscard]] inline const vk::raii::DescriptorSetLayout&
        GetHandle() const { return Handle; }

    private:
        vk::raii::DescriptorSetLayout          Handle {nullptr};

    public:
        FVulkanDescriptorSetLayout() = delete;
        FVulkanDescriptorSetLayout(const vk::raii::Device&                       I_Device,
                                   const TArray<vk::DescriptorSetLayoutBinding>& I_Bindings);
    };

    FVulkanDescriptorSetLayout::
    FVulkanDescriptorSetLayout(const vk::raii::Device&                       I_Device,
                               const TArray<vk::DescriptorSetLayoutBinding>& I_Bindings)
    {
        auto CreateInfo = vk::DescriptorSetLayoutCreateInfo{}
            .setBindingCount    (I_Bindings.size())
            .setPBindings       (I_Bindings.data())
        ;
        auto Result = I_Device.createDescriptorSetLayout(CreateInfo);
        if (Result.has_value())
        { Handle = std::move(*Result); }
        else
        { LOG_FATAL("Failed to create descriptor set layout!"); }
    }
}