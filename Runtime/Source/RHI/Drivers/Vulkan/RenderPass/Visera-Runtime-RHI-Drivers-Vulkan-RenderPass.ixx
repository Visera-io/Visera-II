module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Drivers.Vulkan.RenderPass;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Interface.RenderPass;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanRenderPass : public IRenderPass
    {
    public:
        [[nodiscard]] const void*
        GetHandle() const override { return *Handle; }

    private:
        vk::raii::RenderPass Handle {nullptr};
    };
}
