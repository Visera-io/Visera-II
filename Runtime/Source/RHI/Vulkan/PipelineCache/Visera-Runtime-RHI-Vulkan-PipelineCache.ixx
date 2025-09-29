module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.PipelineCache;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanPipelineCache
    {
    public:
        [[nodiscard]] inline const vk::raii::PipelineCache&
        GetHandle() const { return Handle; }

    private:
        vk::raii::PipelineCache Handle {nullptr};

    public:
        FVulkanPipelineCache();
        ~FVulkanPipelineCache();
    };
}