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
    export class VISERA_RUNTIME_API FVulkanImageView : IVulkanResource
    {
    public:
        [[nodiscard]] inline const vk::raii::ImageView&
        GetHandle() const { return Handle; }

        [[nodiscard]] inline Bool
        IsExpired() const { return Image == nullptr; }

    private:
        vk::raii::ImageView Handle {nullptr};
        const vk::Image&    Image;

    public:
        FVulkanImageView() = delete;
        FVulkanImageView(const vk::Image& I_Image);
    };

    export class VISERA_RUNTIME_API FVulkanImage : IVulkanResource
    {
    public:
        [[nodiscard]] TSharedPtr<FVulkanImageView>
        CreateImageView() const;

        [[nodiscard]] inline const vk::Image&
        GetHandle() const { return Handle; }
        [[nodiscard]] inline EVulkanImageLayout
        GetLayout() const { return Layout; }
        [[nodiscard]] inline EVulkanFormat
        GetFormat() const { return Format; }

        inline void
        ConvertLayout(const vk::raii::CommandBuffer& I_CommandBuffer,
                      const FVulkanImageBarrier&     I_MemoryBarrier);

    protected:
        vk::Image           Handle {nullptr};
        EVulkanImageType    Type   {};
        EVulkanFormat       Format {};
        EVulkanImageLayout  Layout { EVulkanImageLayout::eUndefined };
        FVulkanExtent3D     Extent {0,0,0};
        EVulkanImageUsage   Usages {};

    public:
        FVulkanImage()                               = delete;
        FVulkanImage(EVulkanImageType       I_ImageType,
                     const FVulkanExtent3D&	I_Extent,
                     EVulkanFormat          I_Format,
                     EVulkanImageUsage      I_Usages);
        ~FVulkanImage() override;
        FVulkanImage(const FVulkanImage&)            = delete;
        FVulkanImage& operator=(const FVulkanImage&) = delete;
    };

    export class VISERA_RUNTIME_API FVulkanImageWrapper : public FVulkanImage
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
        ~FVulkanImageWrapper() override { Handle = nullptr; } // Disable release
    };

    FVulkanImageView::
    FVulkanImageView(const vk::Image& I_Image)
    : IVulkanResource{EType::ImageView},
      Image { I_Image }
    {
        VISERA_UNIMPLEMENTED_API;
    }

    FVulkanImage::
    FVulkanImage(EVulkanImageType       I_ImageType,
                 const FVulkanExtent3D&	I_Extent,
                 EVulkanFormat          I_Format,
                 EVulkanImageUsage      I_Usages)
    : IVulkanResource{EType::Image},
      Type   {I_ImageType},
      Format {I_Format},
      Extent {I_Extent},
      Usages {I_Usages}
    {
        auto CreateInfo = vk::ImageCreateInfo{}
            .setImageType               (Type)
            .setExtent                  (Extent)
            .setFormat                  (Format)
            .setMipLevels               (1)
            .setArrayLayers             (1)
            .setSamples                 (EVulkanSamplingRate::e1)
            .setTiling                  (EVulkanImageTiling::eOptimal)
            .setUsage                   (Usages)
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

    TSharedPtr<FVulkanImageView> FVulkanImage::
    CreateImageView() const
    {
        VISERA_UNIMPLEMENTED_API;
        return nullptr;
    }
}