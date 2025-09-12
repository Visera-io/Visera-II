module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Driver.Vulkan.Fence;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Driver.Interface.Fence;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanFence : public IFence
    {
    public:
        const void*
        GetHandle() const override { return *Handle; }

    private:
        vk::raii::Fence Handle {nullptr};

    public:
        FVulkanFence() {}
        ~FVulkanFence() override {}
    };
}
