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
        FVulkanImage(EVulkanImageType       I_ImageType,
                     const FVulkanExtent3D&	I_Extent,
                     EVulkanFormat          I_Format,
                     EVulkanImageUsage      I_Usages);
        ~FVulkanImage()                              { Release(&Handle); };
        FVulkanImage(const FVulkanImage&)            = delete;
        FVulkanImage& operator=(const FVulkanImage&) = delete;
    };

    FVulkanImage::
    FVulkanImage(EVulkanImageType       I_ImageType,
                 const FVulkanExtent3D&	I_Extent,
                 EVulkanFormat          I_Format,
                 EVulkanImageUsage      I_Usages)
    : IVulkanResource{EType::Image}
    {
        auto CreateInfo = vk::ImageCreateInfo{}
            .setImageType               (I_ImageType)
            .setExtent                  (I_Extent)
            .setFormat                  (I_Format)
            .setMipLevels               (1)
            .setArrayLayers             (1)
            .setSamples                 (EVulkanSamplingRate::e1)
            .setTiling                  (EVulkanImageTiling::eOptimal)
            .setUsage                   (I_Usages)
            .setSharingMode             (EVulkanSharingMode::eExclusive)
            .setQueueFamilyIndexCount   (0)
            .setPQueueFamilyIndices     (nullptr)
            .setInitialLayout           (EVulkanImageLayout::eUndefined);
        ;

        Allocate(&Handle, &CreateInfo);
    }

    vk::raii::ImageView FVulkanImage::
    CreateImageView() const
    {
        VISERA_WIP;
        return nullptr;
    }
}