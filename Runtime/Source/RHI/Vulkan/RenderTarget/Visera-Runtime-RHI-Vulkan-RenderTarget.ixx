module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.RenderTarget;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Texture2D;
import Visera.Runtime.RHI.Vulkan.Allocator;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanRenderTarget
    {
    public:
        [[nodiscard]] inline const vk::raii::ImageView&
        GetView() const  { return View; }
        [[nodiscard]] inline const vk::raii::Image&
        GetHandle() const { return Handle; }

    private:
        vk::raii::Image     Handle {nullptr};
        vk::raii::ImageView View {nullptr};

    public:
        FVulkanRenderTarget()                                      = default;
        ~FVulkanRenderTarget()                                     = default;
        FVulkanRenderTarget(const FVulkanRenderTarget&)            = delete;
        FVulkanRenderTarget& operator=(const FVulkanRenderTarget&) = delete;
    };
}
