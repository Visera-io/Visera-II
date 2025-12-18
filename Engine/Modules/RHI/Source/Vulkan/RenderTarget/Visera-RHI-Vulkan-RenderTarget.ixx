module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Vulkan.RenderTarget;
#define VISERA_MODULE_NAME "RHI.Vulkan"
import Visera.RHI.Vulkan.Common;
import Visera.RHI.Vulkan.Image;
import Visera.RHI.Vulkan.Allocator;
import Visera.Runtime.Log;
import vulkan_hpp;

namespace Visera
{
    export class VISERA_RHI_API FVulkanRenderTarget
    {
    public:
        [[nodiscard]] inline TSharedRef<FVulkanImage>
        GetImage() const { return TargetImageView->GetImage(); }
        [[nodiscard]] inline TSharedRef<FVulkanImageView>
        GetImageView() const  { return TargetImageView; }
        [[nodiscard]] inline vk::ImageLayout
        GetLayout() const  { return TargetImageView->GetImage()->GetLayout(); }
        [[nodiscard]] inline vk::Format
        GetFormat() const  { return TargetImageView->GetImage()->GetFormat(); }
        [[nodiscard]] inline vk::AttachmentLoadOp
        GetLoadOp() const { return LoadOp; }
        inline FVulkanRenderTarget*
        SetLoadOp(vk::AttachmentLoadOp I_LoadOp) { LoadOp = I_LoadOp; return this; }
        [[nodiscard]] inline vk::AttachmentStoreOp
        GetStoreOp() const { return StoreOp; }
        inline FVulkanRenderTarget*
        SetStoreOp(vk::AttachmentStoreOp I_StoreOp) { StoreOp = I_StoreOp; return this; }
        [[nodiscard]] inline const vk::ClearColorValue&
        GetClearColor() const { return ClearColor; }
        inline FVulkanRenderTarget&
        SetClearColor(const vk::ClearColorValue& I_ClearColor) { ClearColor = I_ClearColor; return *this; }

        vk::RenderingAttachmentInfo
        GetAttachmentInfo() const;
        [[nodiscard]] inline Bool
        HasDepth() const { return TargetImageView->GetImage()->HasDepth(); }
        [[nodiscard]] inline Bool
        HasStencil() const { return TargetImageView->GetImage()->HasStencil(); }
        void inline
        Swap(TSharedRef<FVulkanRenderTarget> I_Other);

    private:
        TSharedPtr<FVulkanImageView> TargetImageView;

        vk::AttachmentLoadOp    LoadOp     { vk::AttachmentLoadOp::eClear  };
        vk::AttachmentStoreOp   StoreOp    { vk::AttachmentStoreOp::eStore };
        vk::ClearColorValue     ClearColor { vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f) };

    public:
        FVulkanRenderTarget()                                      = delete;
        FVulkanRenderTarget(TSharedPtr<FVulkanImage> I_TargetImage);
        ~FVulkanRenderTarget()                                     = default;
        FVulkanRenderTarget(const FVulkanRenderTarget&)            = delete;
        FVulkanRenderTarget& operator=(const FVulkanRenderTarget&) = delete;
    };

    FVulkanRenderTarget::
    FVulkanRenderTarget(TSharedPtr<FVulkanImage> I_TargetImage)
    {
        VISERA_ASSERT(I_TargetImage && I_TargetImage->GetHandle());
        VISERA_ASSERT(I_TargetImage->GetLayout() != vk::ImageLayout::eUndefined &&
                      "Convert layout before creation for creating view!");
        vk::ImageViewType RTViewType{};
        switch (I_TargetImage->GetType())
        {
        case vk::ImageType::e2D: RTViewType = vk::ImageViewType::e2D; break;
        default: LOG_FATAL("Unsupported image type!"); break;
        }

        vk::ImageAspectFlagBits RTAspect{};
        if (I_TargetImage->HasDepth())
        { RTAspect = vk::ImageAspectFlagBits::eDepth; }
        else if (I_TargetImage->HasStencil())
        { RTAspect = vk::ImageAspectFlagBits::eStencil; }
        else
        { RTAspect = vk::ImageAspectFlagBits::eColor; }

        TargetImageView = MakeShared<FVulkanImageView>(
            I_TargetImage,
            RTViewType,
            RTAspect);
    }

    vk::RenderingAttachmentInfo FVulkanRenderTarget::
    GetAttachmentInfo() const
    {
        return vk::RenderingAttachmentInfo{}
        .setImageView   (GetImageView()->GetHandle())
        .setImageLayout (GetLayout())
        .setLoadOp      (GetLoadOp())
        .setStoreOp     (GetStoreOp())
        .setClearValue  (GetClearColor());
    }

    void FVulkanRenderTarget::
    Swap(TSharedRef<FVulkanRenderTarget> I_Other)
    {
        std::swap(TargetImageView, I_Other->TargetImageView);
        std::swap(LoadOp, I_Other->LoadOp);
        std::swap(StoreOp, I_Other->StoreOp);
        std::swap(ClearColor, I_Other->ClearColor);
    }
}
