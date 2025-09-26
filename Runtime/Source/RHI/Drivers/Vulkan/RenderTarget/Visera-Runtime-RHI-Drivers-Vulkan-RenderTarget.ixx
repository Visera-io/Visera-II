module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Drivers.Vulkan.RenderTarget;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Interface.RenderTarget;
import Visera.Runtime.RHI.Drivers.Vulkan.Texture2D;
import Visera.Runtime.RHI.Drivers.Vulkan.Allocator;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanRenderTarget : public IRenderTarget
    {
    public:


    private:


    public:
        FVulkanRenderTarget()                                      = delete;
        FVulkanRenderTarget(TSharedPtr<FVulkanTexture2D> I_Image);
        virtual ~FVulkanRenderTarget()                             = default;
        FVulkanRenderTarget(const FVulkanRenderTarget&)            = delete;
        FVulkanRenderTarget& operator=(const FVulkanRenderTarget&) = delete;
    };

    FVulkanRenderTarget::
    FVulkanRenderTarget(TSharedPtr<FVulkanTexture2D> I_Image)
    : IRenderTarget{I_Image}
    {

    }
}
