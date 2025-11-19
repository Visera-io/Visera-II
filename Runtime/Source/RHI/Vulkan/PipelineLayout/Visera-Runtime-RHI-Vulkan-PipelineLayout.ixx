module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.PipelineLayout;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Runtime.RHI.Vulkan.DescriptorSet;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanPipelineLayout
    {
    public:
        [[nodiscard]] inline const vk::raii::PipelineLayout&
        GetHandle() const { return Handle; }

    private:
        vk::raii::PipelineLayout        Handle {nullptr};

    public:
        FVulkanPipelineLayout() = delete;
        FVulkanPipelineLayout(const vk::raii::Device&                I_Device,
                              const TArray<vk::DescriptorSetLayout>& I_DescriptorSetLayouts,
                              const TArray<FVulkanPushConstant>&     I_PushConstants);
    };

    FVulkanPipelineLayout::
    FVulkanPipelineLayout(const vk::raii::Device&                I_Device,
                          const TArray<vk::DescriptorSetLayout>& I_DescriptorSetLayouts,
                          const TArray<FVulkanPushConstant>&     I_PushConstants)
    {
        auto PipelineLayoutInfo = vk::PipelineLayoutCreateInfo{}
            .setSetLayoutCount          (I_DescriptorSetLayouts.size())
            .setPSetLayouts             (I_DescriptorSetLayouts.data())
            .setPushConstantRangeCount  (I_PushConstants.size())
            .setPPushConstantRanges     (I_PushConstants.data())
        ;
        auto Result = I_Device.createPipelineLayout(PipelineLayoutInfo);
        if (!Result.has_value())
        { LOG_FATAL("Failed to create the pipeline layout!"); }
        else
        { Handle = std::move(*Result); }
    }
}