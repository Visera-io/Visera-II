module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Vulkan.Sampler;
#define VISERA_MODULE_NAME "RHI.Vulkan"
import Visera.RHI.Vulkan.Common;
import Visera.Global.Log;
import vulkan_hpp;

namespace Visera
{
    export class VISERA_RHI_API FVulkanSampler
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
        FVulkanSampler(const FVulkanSampler&)            = delete;
        FVulkanSampler(FVulkanSampler&&)                 = default;
        FVulkanSampler& operator=(const FVulkanSampler&) = delete;
        FVulkanSampler& operator=(FVulkanSampler&&)      = default;
    };
}