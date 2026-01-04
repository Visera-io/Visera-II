module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Vulkan.Image;
#define VISERA_MODULE_NAME "RHI.Vulkan"
import Visera.RHI.Vulkan.Common;
import Visera.RHI.Vulkan.Allocator;
import Visera.Runtime.Log;
import Visera.Core.Math.Arithmetic.Interval;
import vulkan_hpp;

namespace Visera
{
    export class VISERA_RHI_API FVulkanImage : public IVulkanResource
    {
    public:
        [[nodiscard]] inline const vk::Image&
        GetHandle() const { return Handle; }
        [[nodiscard]] inline vk::Extent3D
        GetExtent() const { return Extent; }
        [[nodiscard]] inline vk::ImageType
        GetType() const { return Type; }
        [[nodiscard]] inline vk::ImageLayout
        GetLayout() const { return Layout; }
        [[nodiscard]] inline vk::Format
        GetFormat() const { return Format; }

        [[nodiscard]] inline Bool
        HasDepth() const;
        [[nodiscard]] inline Bool
        HasStencil() const;

        inline void
        ConvertLayout(const vk::raii::CommandBuffer& I_CommandBuffer,
                      const vk::ImageMemoryBarrier2&     I_MemoryBarrier);

    protected:
        vk::Image           Handle {nullptr};
        vk::ImageType       Type   {};
        vk::Format          Format {};
        vk::ImageLayout     Layout { vk::ImageLayout::eUndefined };
        vk::Extent3D        Extent {0,0,0};
        vk::ImageUsageFlags Usages {};

    public:
        FVulkanImage() : IVulkanResource{EType::Image} {}
        FVulkanImage(vk::ImageType          I_ImageType,
                     const vk::Extent3D&	I_Extent,
                     vk::Format             I_Format,
                     vk::ImageUsageFlags    I_Usages);
        ~FVulkanImage() override;
        FVulkanImage(const FVulkanImage&)            = delete;
        FVulkanImage& operator=(const FVulkanImage&) = delete;
        FVulkanImage(FVulkanImage&& I_Other) noexcept : IVulkanResource{EType::Image}
        {
            Handle = I_Other.Handle;
            Extent = std::move(I_Other.Extent);
            Format = I_Other.Format;
            Layout = I_Other.Layout;
            Usages = I_Other.Usages;
            // Move the VMA allocation from the moved-from object
            GetAllocation() = I_Other.GetAllocation();
            I_Other.GetAllocation() = nullptr;
            I_Other.Handle = nullptr;
        }
        FVulkanImage& operator=(FVulkanImage&& I_Other) noexcept
        {
            if (this != &I_Other)
            {
                // Release current allocation if any
                if (Handle != nullptr)
                {
                    Release(&Handle);
                }
                Handle = I_Other.Handle;
                Extent = std::move(I_Other.Extent);
                Format = I_Other.Format;
                Layout = I_Other.Layout;
                Usages = I_Other.Usages;
                // Move the VMA allocation from the moved-from object
                GetAllocation() = I_Other.GetAllocation();
                I_Other.GetAllocation() = nullptr;
                I_Other.Handle = nullptr;
            }
            return *this;
        }
    };

    export class VISERA_RHI_API FVulkanImageView
    {
    public:
        [[nodiscard]] inline const vk::raii::ImageView&
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
                         TOptional<vk::ComponentMapping>  I_Swizzle     = {});
    };


    export class VISERA_RHI_API FVulkanImageWrapper : public FVulkanImage
    {
    public:
        FVulkanImageWrapper() = delete;
        FVulkanImageWrapper(const vk::Image&        I_Handle,
                            vk::ImageType           I_ImageType,
                            const vk::Extent3D&	    I_Extent,
                            vk::Format              I_Format,
                            vk::ImageUsageFlags     I_Usages)
        {
            VISERA_ASSERT(I_Handle != nullptr);
            Handle = I_Handle;
            Extent = I_Extent;
            Type   = I_ImageType;
            Format = I_Format;
            Usages = I_Usages;
        }
        FVulkanImageWrapper(FVulkanImageWrapper&&) = default;
        FVulkanImageWrapper& operator=(FVulkanImageWrapper&&) = default;
        ~FVulkanImageWrapper() override { Handle = nullptr; } // Disable release
    };

    FVulkanImageView::
    FVulkanImageView(FVulkanImage*                    I_Image,
                     vk::ImageViewType                I_Type,
                     vk::ImageAspectFlags             I_Aspect,
                     TClosedInterval<UInt8>           I_MipmapRange,
                     TClosedInterval<UInt8>           I_ArrayRange,
                     TOptional<vk::ComponentMapping>  I_Swizzle)
    : Image { I_Image }
    {
        VISERA_ASSERT(I_Image != nullptr);
        auto& Device = GVulkanAllocator->GetDevice();

        auto ImageSubresourceRage = vk::ImageSubresourceRange{}
            .setAspectMask      (I_Aspect)
            .setBaseMipLevel    (I_MipmapRange.Left)
            .setLevelCount      (I_MipmapRange.Length() + 1)
            .setBaseArrayLayer  (I_ArrayRange.Left)
            .setLayerCount      (I_ArrayRange.Length() + 1)
        ;
        const auto& Swizzle = I_Swizzle.has_value()?
                            I_Swizzle.value(): IdentitySwizzle;

        const auto CreateInfo = vk::ImageViewCreateInfo{}
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
    FVulkanImage(vk::ImageType          I_ImageType,
                 const vk::Extent3D&	I_Extent,
                 vk::Format             I_Format,
                 vk::ImageUsageFlags    I_Usages)
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
            .setSamples                 (vk::SampleCountFlagBits::e1)
            .setTiling                  (vk::ImageTiling::eOptimal)
            .setUsage                   (Usages)
            .setSharingMode             (vk::SharingMode::eExclusive)
            .setQueueFamilyIndexCount   (0)
            .setPQueueFamilyIndices     (nullptr)
            .setInitialLayout           (vk::ImageLayout::eUndefined);
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
                  const vk::ImageMemoryBarrier2& I_MemoryBarrier)
    {
        auto DependencyInfo = vk::DependencyInfo{}
            .setDependencyFlags         ({})
            .setImageMemoryBarrierCount (1)
            .setPImageMemoryBarriers    (&I_MemoryBarrier)
        ;
        I_CommandBuffer.pipelineBarrier2(DependencyInfo);

        Layout = I_MemoryBarrier.newLayout;
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