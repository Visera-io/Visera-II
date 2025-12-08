module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Vulkan.Pipeline.Layout;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Runtime.RHI.Vulkan.DescriptorSet;
import Visera.Core.Log;
import vulkan_hpp;

namespace Visera
{
    export class VISERA_RUNTIME_API FVulkanPipelineLayout
    {
    public:
        [[nodiscard]] inline const vk::raii::PipelineLayout&
        GetHandle() const { return Handle; }
        [[nodiscard]] inline vk::ShaderStageFlags
        GetPushConstantStages() const { return vk::ShaderStageFlagBits::eAll; } //[TODO]: From set layout / shader reflection
        [[nodiscard]] inline vk::ShaderStageFlags
        GetDescriptorSetStages() const { return vk::ShaderStageFlagBits::eAll; } //[TODO]: From set layout / shader reflection

    private:
        vk::raii::PipelineLayout Handle {nullptr};

    public:
        FVulkanPipelineLayout() = delete;
        FVulkanPipelineLayout(const vk::raii::Device&                I_Device,
                              const TArray<vk::DescriptorSetLayout>& I_DescriptorSetLayouts,
                              const TArray<vk::PushConstantRange>&   I_PushConstants);
    };

    FVulkanPipelineLayout::
    FVulkanPipelineLayout(const vk::raii::Device&                I_Device,
                          const TArray<vk::DescriptorSetLayout>& I_DescriptorSetLayouts,
                          const TArray<vk::PushConstantRange>&   I_PushConstants)
    {
        const auto PipelineLayoutInfo = vk::PipelineLayoutCreateInfo{}
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