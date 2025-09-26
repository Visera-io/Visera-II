module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.Texture2D;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Interface.Texture2D;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanTexture2D : public ITexture2D
    {
    public:
        [[nodiscard]] const void*
        GetHandle() const override { return &Handle; }
        [[nodiscard]] const void*
        GetView()   const override { return &View; }

    private:
        vk::raii::Image     Handle {nullptr};
        vk::raii::ImageView View {nullptr};

    public:
        FVulkanTexture2D()                             = default;
        virtual ~FVulkanTexture2D()                    = default;
        FVulkanTexture2D(const FVulkanTexture2D&)            = delete;
        FVulkanTexture2D& operator=(const FVulkanTexture2D&) = delete;
    };
}