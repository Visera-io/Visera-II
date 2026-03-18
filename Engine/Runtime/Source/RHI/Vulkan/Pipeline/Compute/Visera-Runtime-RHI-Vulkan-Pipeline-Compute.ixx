module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Vulkan.Pipeline.Compute;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.Log;
import Visera.Runtime.RHI.Vulkan.ShaderModule;
import Visera.Runtime.RHI.Vulkan.Pipeline.Layout;
import Visera.Runtime.RHI.Vulkan.Pipeline.Cache;
import vulkan_hpp;

namespace Visera
{
    export class FVulkanComputePipeline
    {
    public:
        [[nodiscard]] inline const vk::raii::Pipeline&
        GetHandle() const { return Handle; }
        [[nodiscard]] inline FVulkanPipelineLayout*
        GetLayout() { return Layout; }
        [[nodiscard]] inline const FVulkanPipelineLayout*
        GetLayout() const { return Layout; }

    private:
        vk::raii::Pipeline      Handle          {nullptr};
        FVulkanPipelineLayout*  Layout          {nullptr};
        FVulkanShaderModule*    ComputeShader   {nullptr};

    public:
        FVulkanComputePipeline() = delete;
        FVulkanComputePipeline(const vk::raii::Device& I_Device,
                               FVulkanPipelineLayout*  I_PipelineLayout,
                               FVulkanShaderModule*    I_ComputeShader,
                               FVulkanPipelineCache*   I_PipelineCache);
    };

    FVulkanComputePipeline::
    FVulkanComputePipeline(const vk::raii::Device& I_Device,
                           FVulkanPipelineLayout*  I_PipelineLayout,
                           FVulkanShaderModule*    I_ComputeShader,
                           FVulkanPipelineCache*   I_PipelineCache)
    : Layout        {I_PipelineLayout},
      ComputeShader {I_ComputeShader}
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
