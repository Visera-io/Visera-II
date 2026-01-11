module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Vulkan.Sync.Semaphore;
#define VISERA_MODULE_NAME "RHI.Vulkan"
import Visera.RHI.Vulkan.Common;
import Visera.Global.Log;
import vulkan_hpp;

namespace Visera
{
    export class VISERA_RHI_API FVulkanSemaphore
    {
    public:
        [[nodiscard]] inline vk::Semaphore
        GetHandle() const { return Handle; }

    private:
        vk::raii::Semaphore Handle {nullptr};

    public:
        FVulkanSemaphore() = default;
        FVulkanSemaphore(const vk::raii::Device& I_Device)
        {
            auto CreateInfo = vk::SemaphoreCreateInfo{};

            auto Result = I_Device.createSemaphore(CreateInfo);
            if (!Result.has_value())
            { LOG_ERROR("Failed to create Vulkan Semaphore!"); }
            else
            { Handle = std::move(*Result); }
        }
        FVulkanSemaphore(FVulkanSemaphore&&) = default;
        FVulkanSemaphore& operator=(FVulkanSemaphore&&) = default;
        ~FVulkanSemaphore() {}
    };
}