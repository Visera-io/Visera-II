module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.RenderTarget;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Runtime.RHI.Vulkan.Image;
import Visera.Runtime.RHI.Vulkan.Allocator;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanRenderTarget
    {
    public:
        [[nodiscard]] inline TSharedRef<FVulkanImageView>
        GetView() const  { return TargetImageView; }
        [[nodiscard]] inline EVulkanImageLayout
        GetLayout() const  { return TargetImage->GetLayout(); }
        [[nodiscard]] inline EVulkanFormat
        GetFormat() const  { return TargetImage->GetFormat(); }
        [[nodiscard]] inline const vk::Image&
        GetHandle() const { return TargetImage->GetHandle(); }
        [[nodiscard]] inline EVulkanLoadOp
        GetLoadOp() const { return LoadOp; }
        inline FVulkanRenderTarget&
        SetLoadOp(EVulkanLoadOp I_LoadOp) { LoadOp = I_LoadOp; return *this; }
        [[nodiscard]] inline EVulkanStoreOp
        GetStoreOp() const { return StoreOp; }
        inline FVulkanRenderTarget&
        SetStoreOp(EVulkanStoreOp I_StoreOp) { StoreOp = I_StoreOp; return *this; }
        [[nodiscard]] inline const FVulkanClearColor&
        GetClearColor() const { return ClearColor; }
        inline FVulkanRenderTarget&
        SetClearColor(const FVulkanClearColor& I_ClearColor) { ClearColor = I_ClearColor; return *this; }

        vk::RenderingAttachmentInfo
        GetAttachmentInfo() const;
        [[nodiscard]] inline Bool
        HasDepth() const;
        [[nodiscard]] inline Bool
        HasStencil() const;
        void inline
        Swap(TSharedRef<FVulkanRenderTarget> I_Other);

    private:
        TSharedPtr<FVulkanImage>     TargetImage;
        TSharedPtr<FVulkanImageView> TargetImageView;

        EVulkanLoadOp       LoadOp     { EVulkanLoadOp::eNone  };
        EVulkanStoreOp      StoreOp    { EVulkanStoreOp::eNone };
        FVulkanClearColor   ClearColor { FVulkanClearColor(0.0f, 0.0f, 0.0f, 1.0f)} ;

    public:
        FVulkanRenderTarget()                                      = delete;
        FVulkanRenderTarget(TSharedPtr<FVulkanImage> I_TargetImage);
        ~FVulkanRenderTarget()                                     = default;
        FVulkanRenderTarget(const FVulkanRenderTarget&)            = delete;
        FVulkanRenderTarget& operator=(const FVulkanRenderTarget&) = delete;
    };

    FVulkanRenderTarget::
    FVulkanRenderTarget(TSharedPtr<FVulkanImage> I_TargetImage)
    : TargetImage{ std::move(I_TargetImage) }
    {
        VISERA_ASSERT(TargetImage && TargetImage->GetHandle());
        VISERA_ASSERT(TargetImage->GetLayout() != EVulkanImageLayout::eUndefined &&
                      "Convert layout before creation!");
    }

    vk::RenderingAttachmentInfo FVulkanRenderTarget::
    GetAttachmentInfo() const
    {
        auto AttachmentInfo = vk::RenderingAttachmentInfo{}
            .setImageView   (GetView()->GetHandle())
            .setImageLayout (GetLayout())
            .setLoadOp      (GetLoadOp())
            .setStoreOp     (GetStoreOp())
            .setClearValue  (GetClearColor())
        ;
        return AttachmentInfo;
    }

    Bool FVulkanRenderTarget::
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

    Bool FVulkanRenderTarget::
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

    void FVulkanRenderTarget::
    Swap(TSharedRef<FVulkanRenderTarget> I_Other)
    {
        std::swap(TargetImage, I_Other->TargetImage);
        std::swap(TargetImageView, I_Other->TargetImageView);
        std::swap(LoadOp, I_Other->LoadOp);
        std::swap(StoreOp, I_Other->StoreOp);
        std::swap(ClearColor, I_Other->ClearColor);
    }
}
