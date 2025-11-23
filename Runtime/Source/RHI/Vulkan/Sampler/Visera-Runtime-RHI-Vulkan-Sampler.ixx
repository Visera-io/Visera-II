module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.Sampler;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_RUNTIME_API FVulkanSampler
    {
    public:
        [[nodiscard]] inline const vk::raii::Sampler&
        GetHandle() const { return Handle; }

    private:
        vk::raii::Sampler Handle {nullptr};

    public:
        FVulkanSampler() = delete;
        FVulkanSampler(const vk::raii::Device&      I_Device,
                       const vk::SamplerCreateInfo& I_CreateInfo)
        {
            auto Result = I_Device.createSampler(I_CreateInfo);
            if (Result.has_value())
            { Handle = std::move(*Result); }
            else
            { LOG_FATAL("Failed to create the Vulkan Sampler!"); }
        }
        ~FVulkanSampler() {}
    };
}