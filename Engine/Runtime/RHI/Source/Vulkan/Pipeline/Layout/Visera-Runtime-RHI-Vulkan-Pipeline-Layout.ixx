module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.Vulkan.Pipeline.Layout;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Runtime.RHI.Vulkan.DescriptorSet;
import Visera.Core.Containers.Array;
import Visera.Core.Log;
import vulkan_hpp;

namespace Visera
{
    export class VISERA_RUNTIME_API FVulkanPipelineLayout
    {
    public:
        [[nodiscard]] inline const vk::raii::PipelineLayout&
        GetHandle() const { return Handle; }
        /** Union of stage flags from all push constant ranges (computed at creation). */
        [[nodiscard]] inline vk::ShaderStageFlags
        GetPushConstantStages() const { return CachedPushConstantStages; }
        [[nodiscard]] inline vk::ShaderStageFlags
        GetDescriptorSetStages() const { return vk::ShaderStageFlagBits::eAll; } //[TODO]: From set layout / shader reflection

    private:
        vk::raii::PipelineLayout Handle {nullptr};
        vk::ShaderStageFlags     CachedPushConstantStages {};

    public:
        FVulkanPipelineLayout() = default;
        FVulkanPipelineLayout(const vk::raii::Device&                I_Device,
                              const TArray<vk::DescriptorSetLayout>& I_DescriptorSetLayouts,
                              const TArray<vk::PushConstantRange>&   I_PushConstants);
        FVulkanPipelineLayout(FVulkanPipelineLayout&&) = default;
        FVulkanPipelineLayout& operator=(FVulkanPipelineLayout&&) = default;
    };

    FVulkanPipelineLayout::
    FVulkanPipelineLayout(const vk::raii::Device&                I_Device,
                          const TArray<vk::DescriptorSetLayout>& I_DescriptorSetLayouts,
                          const TArray<vk::PushConstantRange>&   I_PushConstants)
    {
        for (const auto& R : I_PushConstants)
        { CachedPushConstantStages |= R.stageFlags; }
        const auto PipelineLayoutInfo = vk::PipelineLayoutCreateInfo{}
            .setSetLayoutCount          (I_DescriptorSetLayouts.GetSize())
            .setPSetLayouts             (I_DescriptorSetLayouts.Data())
            .setPushConstantRangeCount  (I_PushConstants.GetSize())
            .setPPushConstantRanges     (I_PushConstants.Data())
        ;
        auto Result = I_Device.createPipelineLayout(PipelineLayoutInfo);
        if (!Result.has_value())
        { LOG_FATAL("Failed to create the pipeline layout!"); }
        else
        { Handle = std::move(*Result); }
    }
}