module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Vulkan.RenderTarget;
#define VISERA_MODULE_NAME "RHI.Vulkan"
import Visera.RHI.Vulkan.Common;
import Visera.RHI.Vulkan.Image;
import Visera.Global.Log;
import vulkan_hpp;

namespace Visera
{
    export class VISERA_RHI_API FVulkanRenderTarget
    {
    public:
        [[nodiscard]] inline FVulkanImageView*
        GetImageView() const { return ImageView; }
        [[nodiscard]] inline FVulkanImage*
        GetImage() const { return ImageView ? ImageView->GetImage() : nullptr; }
        [[nodiscard]] inline vk::AttachmentLoadOp
        GetLoadOp() const { return LoadOp; }
        inline FVulkanRenderTarget&
        SetLoadOp(vk::AttachmentLoadOp I_LoadOp) { LoadOp = I_LoadOp; return *this; }
        [[nodiscard]] inline vk::AttachmentStoreOp
        GetStoreOp() const { return StoreOp; }
        inline FVulkanRenderTarget&
        SetStoreOp(vk::AttachmentStoreOp I_StoreOp) { StoreOp = I_StoreOp; return *this; }
        [[nodiscard]] inline const vk::ClearColorValue&
        GetClearColor() const { return ClearColor; }
        inline FVulkanRenderTarget&
        SetClearColor(const vk::ClearColorValue& I_ClearColor) { ClearColor = I_ClearColor; return *this; }

        vk::RenderingAttachmentInfo
        GetAttachmentInfo() const;
        [[nodiscard]] Bool
        HasDepth()      const { return ImageView->GetImage()->HasDepth();   }
        [[nodiscard]] Bool
        HasStencil()    const { return ImageView->GetImage()->HasStencil(); }

    private:
        FVulkanImageView* ImageView {nullptr};

        vk::AttachmentLoadOp    LoadOp     { vk::AttachmentLoadOp::eClear  };
        vk::AttachmentStoreOp   StoreOp    { vk::AttachmentStoreOp::eStore };
        vk::ClearColorValue     ClearColor { vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f) };

    public:
        FVulkanRenderTarget()                                      = default;
        FVulkanRenderTarget(FVulkanImageView* I_ImageView);
        FVulkanRenderTarget(FVulkanRenderTarget&&)                 = default;
        FVulkanRenderTarget& operator=(FVulkanRenderTarget&&)      = default;
        ~FVulkanRenderTarget()                                     = default;
        FVulkanRenderTarget(const FVulkanRenderTarget&)            = delete;
        FVulkanRenderTarget& operator=(const FVulkanRenderTarget&) = delete;
    };

    FVulkanRenderTarget::
    FVulkanRenderTarget(FVulkanImageView* I_ImageView)
    : ImageView { I_ImageView }
    {
        VISERA_ASSERT(I_ImageView != nullptr);
        VISERA_ASSERT(I_ImageView->GetImage() != nullptr);
    }

    vk::RenderingAttachmentInfo FVulkanRenderTarget::
    GetAttachmentInfo() const
    {
        const auto* Image = ImageView->GetImage();
        return vk::RenderingAttachmentInfo{}
        .setImageView   (ImageView->GetHandle())
        .setImageLayout (Image->GetLayout())
        .setLoadOp      (GetLoadOp())
        .setStoreOp     (GetStoreOp())
        .setClearValue  (GetClearColor());
    }
}
