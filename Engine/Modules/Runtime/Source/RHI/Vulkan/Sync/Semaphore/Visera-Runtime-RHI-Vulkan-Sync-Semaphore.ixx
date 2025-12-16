module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Vulkan.Sync.Semaphore;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Runtime.Log;
import vulkan_hpp;

namespace Visera
{
    export class VISERA_RUNTIME_API FVulkanSemaphore
    {
    public:
        [[nodiscard]] inline const vk::raii::Semaphore&
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
        ~FVulkanSemaphore() {}
    };
}