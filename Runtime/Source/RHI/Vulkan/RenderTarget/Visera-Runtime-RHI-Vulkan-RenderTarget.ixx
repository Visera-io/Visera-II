module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.RenderTarget;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Interface.RenderTarget;
import Visera.Runtime.RHI.Vulkan.Texture2D;
import Visera.Runtime.RHI.Vulkan.Allocator;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanRenderTarget : public IRenderTarget
    {
    public:
        [[nodiscard]] const void*
        GetView() const override  { return nullptr; }
        [[nodiscard]] const void*
        GetHandle() const override { return nullptr; }

    private:


    public:
        FVulkanRenderTarget()                                      = default;
        virtual ~FVulkanRenderTarget()                             = default;
        FVulkanRenderTarget(const FVulkanRenderTarget&)            = delete;
        FVulkanRenderTarget& operator=(const FVulkanRenderTarget&) = delete;
    };
}
