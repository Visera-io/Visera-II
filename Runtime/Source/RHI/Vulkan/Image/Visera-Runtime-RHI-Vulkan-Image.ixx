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

        inline void
        ConvertLayout(const vk::raii::CommandBuffer& I_CommandBuffer,
                      const FVulkanImageBarrier&     I_MemoryBarrier);

    protected:
        vk::Image           Handle {nullptr};
        EVulkanImageLayout  Layout { EVulkanImageLayout::eUndefined };

    public:
        FVulkanImage()                               = delete;
        FVulkanImage(EVulkanImageType       I_ImageType,
                     const FVulkanExtent3D&	I_Extent,
                     EVulkanFormat          I_Format,
                     EVulkanImageUsage      I_Usages);
        ~FVulkanImage();
        FVulkanImage(const FVulkanImage&)            = delete;
        FVulkanImage& operator=(const FVulkanImage&) = delete;
    };

    export class FVulkanImageWrapper : public FVulkanImage
    {
    public:
        FVulkanImageWrapper() = delete;
        FVulkanImageWrapper(const vk::Image&        I_Handle,
                            EVulkanImageType        I_ImageType,
                            const FVulkanExtent3D&	I_Extent,
                            EVulkanFormat           I_Format,
                            EVulkanImageUsage       I_Usages)
        : FVulkanImage{I_ImageType, I_Extent, I_Format, I_Usages}
        {
            Handle = I_Handle;
        }
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

    FVulkanImage::
    ~FVulkanImage()
    {
        Release(&Handle);
    }

    void FVulkanImage::
    ConvertLayout(const vk::raii::CommandBuffer& I_CommandBuffer,
                  const FVulkanImageBarrier&     I_MemoryBarrier)
    {
        auto DependencyInfo = vk::DependencyInfo{}
            .setDependencyFlags     ({})
            .setImageMemoryBarrierCount(1)
            .setPImageMemoryBarriers(&I_MemoryBarrier)
        ;
        I_CommandBuffer.pipelineBarrier2(DependencyInfo);

        Layout = I_MemoryBarrier.newLayout;
    }

    vk::raii::ImageView FVulkanImage::
    CreateImageView() const
    {
        VISERA_UNIMPLEMENTED_API;
        return nullptr;
    }
}