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
    export class VISERA_RUNTIME_API FVulkanImage : public IVulkanResource
    {
    public:
        [[nodiscard]] inline const vk::Image&
        GetHandle() const { return Handle; }
        [[nodiscard]] inline FVulkanExtent3D
        GetExtent() const { return Extent; }
        [[nodiscard]] inline EVulkanImageType
        GetType() const { return Type; }
        [[nodiscard]] inline EVulkanImageLayout
        GetLayout() const { return Layout; }
        [[nodiscard]] inline EVulkanFormat
        GetFormat() const { return Format; }

        [[nodiscard]] inline Bool
        HasDepth() const;
        [[nodiscard]] inline Bool
        HasStencil() const;

        inline void
        ConvertLayout(const vk::raii::CommandBuffer& I_CommandBuffer,
                      const FVulkanImageBarrier&     I_MemoryBarrier);

    protected:
        vk::Image           Handle {nullptr};
        EVulkanImageType    Type   {};
        EVulkanFormat       Format {};
        EVulkanImageLayout  Layout { EVulkanImageLayout::eUndefined };
        FVulkanExtent3D     Extent {0,0,0};
        EVulkanImageUsages  Usages {};

    public:
        FVulkanImage() : IVulkanResource{EType::Image} {}
        FVulkanImage(EVulkanImageType       I_ImageType,
                     const FVulkanExtent3D&	I_Extent,
                     EVulkanFormat          I_Format,
                     EVulkanImageUsages     I_Usages);
        ~FVulkanImage() override;
        FVulkanImage(const FVulkanImage&)            = delete;
        FVulkanImage& operator=(const FVulkanImage&) = delete;
    };

    export class VISERA_RUNTIME_API FVulkanImageView
    {
    public:
        [[nodiscard]] inline const vk::raii::ImageView&
        GetHandle() const { return Handle; }

        [[nodiscard]] inline Bool
        IsExpired() const { return Image.expired(); }

    private:
        vk::raii::ImageView    Handle {nullptr};
        TWeakPtr<FVulkanImage> Image;

    public:
        FVulkanImageView() = delete;
        FVulkanImageView(TSharedPtr<FVulkanImage>   I_Image,
                         EVulkanImageViewType       I_Type,
                         EVulkanImageAspect         I_Aspect,
                         const TPair<UInt8, UInt8>& I_MipmapRange = {0,0},
                         const TPair<UInt8, UInt8>& I_ArrayRange  = {0,0},
                         TOptional<FVulkanSwizzle>  I_Swizzle     = {});
    };


    export class VISERA_RUNTIME_API FVulkanImageWrapper : public FVulkanImage
    {
    public:
        FVulkanImageWrapper() = delete;
        FVulkanImageWrapper(const vk::Image&        I_Handle,
                            EVulkanImageType        I_ImageType,
                            const FVulkanExtent3D&	I_Extent,
                            EVulkanFormat           I_Format,
                            EVulkanImageUsages      I_Usages)
        {
            Handle = I_Handle;
            Extent = I_Extent;
            Type   = I_ImageType;
            Format = I_Format;
            Usages = I_Usages;
        }
        ~FVulkanImageWrapper() override { Handle = nullptr; } // Disable release
    };

    FVulkanImageView::
    FVulkanImageView(TSharedPtr<FVulkanImage>   I_Image,
                     EVulkanImageViewType       I_Type,
                     EVulkanImageAspect         I_Aspect,
                     const TPair<UInt8, UInt8>& I_MipmapRange,
                     const TPair<UInt8, UInt8>& I_ArrayRange,
                     TOptional<FVulkanSwizzle>  I_Swizzle)
    : Image {I_Image}
    {
        auto& Device = GVulkanAllocator->GetDevice();

        auto ImageSubresourceRage = vk::ImageSubresourceRange{}
            .setAspectMask(I_Aspect)
            .setBaseMipLevel(I_MipmapRange.first)
            .setLevelCount(I_MipmapRange.second - I_MipmapRange.first + 1)
            .setBaseArrayLayer(I_ArrayRange.first)
            .setLayerCount(I_ArrayRange.second - I_ArrayRange.first + 1)
        ;
        const auto& Swizzle = I_Swizzle.has_value()?
                            I_Swizzle.value(): IdentitySwizzle;

        auto CreateInfo = vk::ImageViewCreateInfo{}
            .setImage            (I_Image->GetHandle())
            .setViewType         (I_Type)
            .setFormat           (I_Image->GetFormat())
            .setComponents       (Swizzle)
            .setSubresourceRange (ImageSubresourceRage)
        ;
        auto Result = Device.createImageView(CreateInfo);
        if (!Result.has_value())
        { LOG_FATAL("Failed to create Vulkan Image View"); }
        else
        { Handle = std::move(*Result); }
    }

    FVulkanImage::
    FVulkanImage(EVulkanImageType       I_ImageType,
                 const FVulkanExtent3D&	I_Extent,
                 EVulkanFormat          I_Format,
                 EVulkanImageUsages     I_Usages)
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
    
    Bool FVulkanImage::
    HasDepth() const
    {
        switch (GetFormat())
        {
            case EVulkanFormat::eD16Unorm:
            case EVulkanFormat::eX8D24UnormPack32:
            case EVulkanFormat::eD32Sfloat:
            case EVulkanFormat::eD16UnormS8Uint:
            case EVulkanFormat::eD24UnormS8Uint:
            case EVulkanFormat::eD32SfloatS8Uint:
                return True;
            default:
                return False;
        }
    }

    Bool FVulkanImage::
    HasStencil() const
    {
        switch (GetFormat())
        {
            case EVulkanFormat::eS8Uint:
            case EVulkanFormat::eD16UnormS8Uint:
            case EVulkanFormat::eD24UnormS8Uint:
            case EVulkanFormat::eD32SfloatS8Uint:
                return true;
            default:
                return false;
        }
    }
}