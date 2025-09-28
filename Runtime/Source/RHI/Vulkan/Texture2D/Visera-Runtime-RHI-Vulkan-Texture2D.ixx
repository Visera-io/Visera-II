module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.Texture2D;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanTexture2D
    {
    public:
        [[nodiscard]] inline const vk::raii::Image&
        GetHandle() const { return Handle; }
        [[nodiscard]] inline const vk::raii::ImageView&
        GetView()   const { return View; }
        [[nodiscard]] inline EVulkanImageLayout
        GetLayout() const { return Layout; }

    private:
        vk::raii::Image     Handle {nullptr};
        vk::raii::ImageView View {nullptr};
        EVulkanImageLayout  Layout { EVulkanImageLayout::eUndefined };

    public:
        FVulkanTexture2D()                             = default;
        virtual ~FVulkanTexture2D()                    = default;
        FVulkanTexture2D(const FVulkanTexture2D&)            = delete;
        FVulkanTexture2D& operator=(const FVulkanTexture2D&) = delete;
    };
}