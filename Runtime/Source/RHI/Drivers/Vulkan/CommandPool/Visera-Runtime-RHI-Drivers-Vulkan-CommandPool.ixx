module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Drivers.Vulkan.CommandPool;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Interface.CommandPool;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanCommandPool : public ICommandPool
    {
    public:
        const void*
        GetHandle() const override { return &Handle; }

    private:
        vk::raii::CommandPool Handle {nullptr};

    public:
        FVulkanCommandPool() {}
        ~FVulkanCommandPool() override {}
    };
}
