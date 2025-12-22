module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Vulkan.Pipeline.Compute;
#define VISERA_MODULE_NAME "RHI.Vulkan"
import Visera.Runtime.Log;
import Visera.RHI.Vulkan.ShaderModule;
import Visera.RHI.Vulkan.Pipeline.Layout;
import Visera.RHI.Vulkan.Pipeline.Cache;
import vulkan_hpp;

namespace Visera
{
    export class FVulkanComputePipeline
    {
    public:
        [[nodiscard]] inline const vk::raii::Pipeline&
        GetHandle() const { return Handle; }
        [[nodiscard]] inline const TSharedPtr<FVulkanPipelineLayout>&
        GetLayout() const { return Layout; }

    private:
        vk::raii::Pipeline                Handle{nullptr};
        TSharedPtr<FVulkanPipelineLayout> Layout{nullptr};
        TSharedRef<FVulkanShaderModule>   ComputeShader{nullptr};

    public:
        FVulkanComputePipeline() = delete;
        FVulkanComputePipeline(const vk::raii::Device&           I_Device,
                               TSharedRef<FVulkanPipelineLayout> I_PipelineLayout,
                               TSharedRef<FVulkanShaderModule>   I_ComputeShader,
                               TUniqueRef<FVulkanPipelineCache>  I_PipelineCache);
    };

    FVulkanComputePipeline::
    FVulkanComputePipeline(const vk::raii::Device&           I_Device,
                           TSharedRef<FVulkanPipelineLayout> I_PipelineLayout,
                           TSharedRef<FVulkanShaderModule>   I_ComputeShader,
                           TUniqueRef<FVulkanPipelineCache>  I_PipelineCache)
    : ComputeShader {I_ComputeShader},
      Layout        {I_PipelineLayout}
    {
        const auto ShaderStageCreateInfo = vk::PipelineShaderStageCreateInfo{}
            .setStage  (vk::ShaderStageFlagBits::eCompute)
            .setPName  (ComputeShader->GetEntryPoint())
            .setModule (ComputeShader->GetHandle())
        ;
        const auto CreateInfo = vk::ComputePipelineCreateInfo{}
            .setStage               (ShaderStageCreateInfo)
            .setLayout              (Layout->GetHandle())
            .setBasePipelineHandle  (nullptr)
            .setBasePipelineIndex   (-1)
            .setPNext               (nullptr)
        ;
        auto Result = I_Device.createComputePipeline(I_PipelineCache->GetHandle(), CreateInfo);
        if (Result.has_value())
        { Handle = std::move(*Result); }
        else
        { LOG_FATAL("Failed to create the compute pipeline!"); }
    }
}
