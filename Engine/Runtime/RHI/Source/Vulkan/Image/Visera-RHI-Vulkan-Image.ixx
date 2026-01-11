module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Vulkan.Image;
#define VISERA_MODULE_NAME "RHI.Vulkan"
import Visera.RHI.Vulkan.Common;
import Visera.RHI.Vulkan.Allocator;
import Visera.Global.Log;
import Visera.Core.Math.Arithmetic.Interval;
import vulkan_hpp;

namespace Visera
{
    export class FVulkanImage;

    export class VISERA_RHI_API FVulkanImageView
    {
    public:
        [[nodiscard]] inline vk::ImageView
        GetHandle() const { return Handle; }
        [[nodiscard]] inline FVulkanImage*
        GetImage() const { return Image; }

    private:
        vk::raii::ImageView      Handle {nullptr};
        FVulkanImage*            Image  {nullptr};

    public:
        FVulkanImageView() = default;
        FVulkanImageView(FVulkanImage*                    I_Image,  // Needed for construction only
                         vk::ImageViewType                I_Type,
                         vk::ImageAspectFlags             I_Aspect,
                         TClosedInterval<UInt8>           I_MipmapRange = {0,0},
                         TClosedInterval<UInt8>           I_ArrayRange  = {0,0},
                         const vk::ComponentMapping&      I_Swizzle     = {});
    };


    export class VISERA_RHI_API FVulkanImage : public IVulkanResource
    {
    public:
        [[nodiscard]] inline const vk::Image&
        GetHandle() const { return Handle; }
        [[nodiscard]] inline const vk::Extent3D&
        GetExtent() const { return Info.extent; }
        [[nodiscard]] inline vk::ImageType
        GetType() const { return Info.imageType; }
        [[nodiscard]] inline vk::ImageLayout
        GetLayout() const { return CurrentLayout; }
        [[nodiscard]] inline vk::Format
        GetFormat() const { return Info.format; }
        [[nodiscard]] inline UInt8
        GetMipmapLevels() const { return Info.mipLevels; }
        [[nodiscard]] inline UInt8
        GetArrayLayers() const { return Info.arrayLayers; }

        [[nodiscard]] inline Bool
        HasDepth() const;
        [[nodiscard]] inline Bool
        HasStencil() const;

        inline void
        ConvertLayout(vk::CommandBuffer              I_CommandBuffer,
                      const vk::ImageMemoryBarrier2& I_MemoryBarrier);

    protected:
        vk::Image           Handle {nullptr};
        vk::ImageCreateInfo Info;
        vk::ImageLayout     CurrentLayout {vk::ImageLayout::eUndefined};

    public:
        FVulkanImage() : IVulkanResource{nullptr, EType::Image} {}
        FVulkanImage(TUniqueRef<FVulkanAllocator> I_Allocator,
                     const vk::ImageCreateInfo&   I_CreateInfo,
                     EVMAMemoryProperty           I_MemoryProperties);
        FVulkanImage(TUniqueRef<FVulkanAllocator> I_Allocator)
        : IVulkanResource{I_Allocator.get(), EType::SwapChainImage} {}
        ~FVulkanImage() override;
        FVulkanImage(const FVulkanImage&)            = delete;
        FVulkanImage& operator=(const FVulkanImage&) = delete;
        FVulkanImage(FVulkanImage&& I_Other) noexcept
        : IVulkanResource(std::move(I_Other)),
          Handle    (std::exchange(I_Other.Handle, vk::Image{})),
          Info      (std::move(I_Other.Info))
        { }

        FVulkanImage& operator=(FVulkanImage&& I_Other) noexcept
        {
            if (this != &I_Other)
            {
                if (Handle != nullptr) { Release(&Handle); }
                IVulkanResource::operator=(std::move(I_Other));
                Handle = std::exchange(I_Other.Handle, vk::Image{});
                Info   = std::move(I_Other.Info);
            }
            return *this;
        }
    };

    export class VISERA_RHI_API FVulkanSwapChainImage : public FVulkanImage
    {
    public:
        FVulkanSwapChainImage() = default;
        FVulkanSwapChainImage(TUniqueRef<FVulkanAllocator> I_Allocator,
                              const vk::Image&             I_Handle,
                              vk::ImageType                I_ImageType,
                              const vk::Extent3D&	       I_Extent,
                              vk::Format                   I_Format,
                              vk::ImageUsageFlags          I_Usages)
        : FVulkanImage(I_Allocator)
        {
            VISERA_ASSERT(I_Handle != nullptr);
            Handle           = I_Handle;
            Info.extent      = I_Extent;
            Info.imageType   = I_ImageType;
            Info.format      = I_Format;
            Info.usage       = I_Usages;
            Info.mipLevels   = 1;
            Info.arrayLayers = 1;
        }
        FVulkanSwapChainImage(FVulkanSwapChainImage&& I_Other) noexcept = default;
        FVulkanSwapChainImage& operator=(FVulkanSwapChainImage&& I_Other) noexcept = default;
        ~FVulkanSwapChainImage() override { Handle = nullptr; } // Disable release
    };

    FVulkanImageView::
    FVulkanImageView(FVulkanImage*                    I_Image,
                     vk::ImageViewType                I_Type,
                     vk::ImageAspectFlags             I_Aspect,
                     TClosedInterval<UInt8>           I_MipmapRange,
                     TClosedInterval<UInt8>           I_ArrayRange,
                     const vk::ComponentMapping&      I_Swizzle)
    : Image { I_Image }
    {
        VISERA_ASSERT(I_Image != nullptr);
        VISERA_ASSERT(I_MipmapRange.Contains(I_Image->GetMipmapLevels() - 1));
        VISERA_ASSERT(I_ArrayRange.Contains(I_Image->GetArrayLayers() - 1));

        auto& Device = Image->GetAllocator()->GetDevice();

        const auto ImageSubresourceRage = vk::ImageSubresourceRange{}
            .setAspectMask      (I_Aspect)
            .setBaseMipLevel    (I_MipmapRange.Left)
            .setLevelCount      (I_MipmapRange.Length() + 1)
            .setBaseArrayLayer  (I_ArrayRange.Left)
            .setLayerCount      (I_ArrayRange.Length() + 1)
        ;
        const auto CreateInfo = vk::ImageViewCreateInfo{}
            .setImage            (I_Image->GetHandle())
            .setViewType         (I_Type)
            .setFormat           (I_Image->GetFormat())
            .setComponents       (I_Swizzle)
            .setSubresourceRange (ImageSubresourceRage)
        ;
        auto Result = Device.createImageView(CreateInfo);
        if (!Result.has_value())
        { LOG_FATAL("Failed to create Vulkan Image View"); }
        else
        { Handle = std::move(*Result); }
    }

    FVulkanImage::
    FVulkanImage(TUniqueRef<FVulkanAllocator> I_Allocator,
                 const vk::ImageCreateInfo&   I_CreateInfo,
                 EVMAMemoryProperty           I_MemoryProperties)
    : IVulkanResource   {I_Allocator.get(), EType::Image},
      Info              { I_CreateInfo },
      CurrentLayout     { I_CreateInfo.initialLayout }
    {
        VISERA_ASSERT(Info.initialLayout == vk::ImageLayout::eUndefined);
        Allocate(&Handle, &Info, nullptr, I_MemoryProperties);
    }

    FVulkanImage::
    ~FVulkanImage()
    {
        Release(&Handle);
    }

    void FVulkanImage::
    ConvertLayout(vk::CommandBuffer              I_CommandBuffer,
                  const vk::ImageMemoryBarrier2& I_MemoryBarrier)
    {
        auto DependencyInfo = vk::DependencyInfo{}
            .setDependencyFlags         ({})
            .setImageMemoryBarrierCount (1)
            .setPImageMemoryBarriers    (&I_MemoryBarrier)
        ;
        I_CommandBuffer.pipelineBarrier2(DependencyInfo);

        CurrentLayout = I_MemoryBarrier.newLayout;
    }
    
    Bool FVulkanImage::
    HasDepth() const
    {
        switch (GetFormat())
        {
            case vk::Format::eD16Unorm:
            case vk::Format::eX8D24UnormPack32:
            case vk::Format::eD32Sfloat:
            case vk::Format::eD16UnormS8Uint:
            case vk::Format::eD24UnormS8Uint:
            case vk::Format::eD32SfloatS8Uint:
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
            case vk::Format::eS8Uint:
            case vk::Format::eD16UnormS8Uint:
            case vk::Format::eD24UnormS8Uint:
            case vk::Format::eD32SfloatS8Uint:
                return true;
            default:
                return false;
        }
    }
}