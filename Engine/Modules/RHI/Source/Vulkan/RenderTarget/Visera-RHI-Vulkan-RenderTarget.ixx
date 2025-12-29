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

    private:
        TSharedPtr<FVulkanImageView> TargetImageView;

        vk::AttachmentLoadOp    LoadOp     { vk::AttachmentLoadOp::eClear  };
        vk::AttachmentStoreOp   StoreOp    { vk::AttachmentStoreOp::eStore };
        vk::ClearColorValue     ClearColor { vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f) };

    public:
        FVulkanRenderTarget()                                      = delete;
        FVulkanRenderTarget(TSharedRef<FVulkanImageView> I_ImageView);
        ~FVulkanRenderTarget()                                     = default;
        FVulkanRenderTarget(const FVulkanRenderTarget&)            = delete;
        FVulkanRenderTarget& operator=(const FVulkanRenderTarget&) = delete;
    };

    FVulkanRenderTarget::
    FVulkanRenderTarget(TSharedRef<FVulkanImageView> I_ImageView)
    : TargetImageView { I_ImageView }
    {

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
}
