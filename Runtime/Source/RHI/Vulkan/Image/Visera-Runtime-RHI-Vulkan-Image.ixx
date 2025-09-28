module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.Image;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Runtime.RHI.Vulkan.Allocator;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanImage : IVulkanResource
    {
    public:
        [[nodiscard]] vk::raii::ImageView
        CreateImageView() const;

        [[nodiscard]] inline const vk::Image&
        GetHandle() const { return Handle; }
        [[nodiscard]] inline EVulkanImageLayout
        GetLayout() const { return Layout; }

        // CommandBuffer API !!!
        inline void
        SetLayout(EVulkanImageLayout I_NewLayout) { Layout = I_NewLayout; }

    private:
        vk::Image           Handle {nullptr};
        EVulkanImageLayout  Layout { EVulkanImageLayout::eUndefined };

    public:
        FVulkanImage()                               = delete;
        FVulkanImage(TUniqueRef<FVulkanAllocator> I_Allocator,
                     FVulkanExtent3D              I_Extent,
                     EVulkanImageLayout           I_Layout);
        ~FVulkanImage()                              = default;
        FVulkanImage(const FVulkanImage&)            = delete;
        FVulkanImage& operator=(const FVulkanImage&) = delete;
    };

    FVulkanImage::
    FVulkanImage(TUniqueRef<FVulkanAllocator> I_Allocator,
                     FVulkanExtent3D          I_Extent,
                     EVulkanImageLayout       I_Layout)
    : IVulkanResource{EType::Image}
    {

    }

    vk::raii::ImageView FVulkanImage::
    CreateImageView() const
    {
        VISERA_WIP;
        return nullptr;
    }
}