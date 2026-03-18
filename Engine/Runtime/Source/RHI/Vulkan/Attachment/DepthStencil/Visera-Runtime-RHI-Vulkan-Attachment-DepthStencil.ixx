module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Vulkan.Attachment.DepthStencil;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Runtime.RHI.Vulkan.Image;
import Visera.Core.Log;
import vulkan_hpp;

namespace Visera
{
    /** Depth/stencil attachment for VkDynamicRendering; produces VkRenderingAttachmentInfo with clearValue.depthStencil. */
    export class VISERA_RUNTIME_API FVulkanDepthStencilAttachment
    {
    public:
        [[nodiscard]] inline FVulkanImageView*
        GetImageView() const { return ImageView; }
        [[nodiscard]] inline vk::AttachmentLoadOp
        GetDepthLoadOp() const { return DepthLoadOp; }
        inline FVulkanDepthStencilAttachment&
        SetDepthLoadOp(vk::AttachmentLoadOp I_LoadOp) { DepthLoadOp = I_LoadOp; return *this; }
        [[nodiscard]] inline vk::AttachmentStoreOp
        GetDepthStoreOp() const { return DepthStoreOp; }
        inline FVulkanDepthStencilAttachment&
        SetDepthStoreOp(vk::AttachmentStoreOp I_StoreOp) { DepthStoreOp = I_StoreOp; return *this; }
        [[nodiscard]] inline vk::AttachmentLoadOp
        GetStencilLoadOp() const { return StencilLoadOp; }
        inline FVulkanDepthStencilAttachment&
        SetStencilLoadOp(vk::AttachmentLoadOp I_LoadOp) { StencilLoadOp = I_LoadOp; return *this; }
        [[nodiscard]] inline vk::AttachmentStoreOp
        GetStencilStoreOp() const { return StencilStoreOp; }
        inline FVulkanDepthStencilAttachment&
        SetStencilStoreOp(vk::AttachmentStoreOp I_StoreOp) { StencilStoreOp = I_StoreOp; return *this; }
        [[nodiscard]] inline const vk::ClearDepthStencilValue&
        GetClearDepthStencil() const { return ClearDepthStencil; }
        inline FVulkanDepthStencilAttachment&
        SetClearDepthStencil(const vk::ClearDepthStencilValue& I_Value) { ClearDepthStencil = I_Value; return *this; }

        /** Fills VkRenderingAttachmentInfo for depth (same imageView, depth load/store, shared clearValue.depthStencil). */
        [[nodiscard]] vk::RenderingAttachmentInfo
        GetDepthAttachmentInfo(vk::ImageLayout I_ImageLayout) const;
        /** Fills VkRenderingAttachmentInfo for stencil (same imageView, stencil load/store, shared clearValue.depthStencil). */
        [[nodiscard]] vk::RenderingAttachmentInfo
        GetStencilAttachmentInfo(vk::ImageLayout I_ImageLayout) const;

    private:
        FVulkanImageView* ImageView {nullptr};

        vk::AttachmentLoadOp         DepthLoadOp    { vk::AttachmentLoadOp::eClear };
        vk::AttachmentStoreOp        DepthStoreOp   { vk::AttachmentStoreOp::eStore };
        vk::AttachmentLoadOp         StencilLoadOp  { vk::AttachmentLoadOp::eDontCare };
        vk::AttachmentStoreOp        StencilStoreOp { vk::AttachmentStoreOp::eDontCare };
        vk::ClearDepthStencilValue   ClearDepthStencil { 1.0f, 0u };

    public:
        FVulkanDepthStencilAttachment()                                            = default;
        explicit FVulkanDepthStencilAttachment(FVulkanImageView* I_ImageView);
        FVulkanDepthStencilAttachment(FVulkanDepthStencilAttachment&&)             = default;
        FVulkanDepthStencilAttachment& operator=(FVulkanDepthStencilAttachment&&)  = default;
        ~FVulkanDepthStencilAttachment()                                           = default;
        FVulkanDepthStencilAttachment(const FVulkanDepthStencilAttachment&)       = delete;
        FVulkanDepthStencilAttachment& operator=(const FVulkanDepthStencilAttachment&) = delete;
    };

    FVulkanDepthStencilAttachment::
    FVulkanDepthStencilAttachment(FVulkanImageView* I_ImageView)
    : ImageView { I_ImageView }
    {
        VISERA_ASSERT(I_ImageView != nullptr);
    }

    vk::RenderingAttachmentInfo FVulkanDepthStencilAttachment::
    GetDepthAttachmentInfo(vk::ImageLayout I_ImageLayout) const
    {
        vk::ClearValue ClearValue;
        ClearValue.depthStencil = GetClearDepthStencil();
        return vk::RenderingAttachmentInfo{}
            .setImageView   (ImageView->GetHandle())
            .setImageLayout (I_ImageLayout)
            .setLoadOp      (GetDepthLoadOp())
            .setStoreOp     (GetDepthStoreOp())
            .setClearValue  (ClearValue);
    }

    vk::RenderingAttachmentInfo FVulkanDepthStencilAttachment::
    GetStencilAttachmentInfo(vk::ImageLayout I_ImageLayout) const
    {
        vk::ClearValue ClearValue;
        ClearValue.depthStencil = GetClearDepthStencil();
        return vk::RenderingAttachmentInfo{}
            .setImageView   (ImageView->GetHandle())
            .setImageLayout (I_ImageLayout)
            .setLoadOp      (GetStencilLoadOp())
            .setStoreOp     (GetStencilStoreOp())
            .setClearValue  (ClearValue);
    }
}
