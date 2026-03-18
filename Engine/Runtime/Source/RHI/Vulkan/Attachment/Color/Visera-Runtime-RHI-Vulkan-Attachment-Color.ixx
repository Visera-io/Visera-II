module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Vulkan.Attachment.Color;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Runtime.RHI.Vulkan.Image;
import Visera.Core.Log;
import vulkan_hpp;

namespace Visera
{
    /** Color attachment for VkDynamicRendering; produces VkRenderingAttachmentInfo with clearValue.color. */
    export class VISERA_RUNTIME_API FVulkanColorAttachment
    {
    public:
        [[nodiscard]] inline FVulkanImageView*
        GetImageView() const { return ImageView; }
        [[nodiscard]] inline vk::AttachmentLoadOp
        GetLoadOp() const { return LoadOp; }
        inline FVulkanColorAttachment&
        SetLoadOp(vk::AttachmentLoadOp I_LoadOp) { LoadOp = I_LoadOp; return *this; }
        [[nodiscard]] inline vk::AttachmentStoreOp
        GetStoreOp() const { return StoreOp; }
        inline FVulkanColorAttachment&
        SetStoreOp(vk::AttachmentStoreOp I_StoreOp) { StoreOp = I_StoreOp; return *this; }
        [[nodiscard]] inline const vk::ClearColorValue&
        GetClearColor() const { return ClearColor; }
        inline FVulkanColorAttachment&
        SetClearColor(const vk::ClearColorValue& I_ClearColor) { ClearColor = I_ClearColor; return *this; }

        [[nodiscard]] vk::RenderingAttachmentInfo
        GetAttachmentInfo(vk::ImageLayout I_ImageLayout) const;

    private:
        FVulkanImageView* ImageView {nullptr};

        vk::AttachmentLoadOp    LoadOp     { vk::AttachmentLoadOp::eClear  };
        vk::AttachmentStoreOp   StoreOp    { vk::AttachmentStoreOp::eStore };
        vk::ClearColorValue     ClearColor { vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f) };

    public:
        FVulkanColorAttachment()                                        = default;
        explicit FVulkanColorAttachment(FVulkanImageView* I_ImageView);
        FVulkanColorAttachment(FVulkanColorAttachment&&)                = default;
        FVulkanColorAttachment& operator=(FVulkanColorAttachment&&)    = default;
        ~FVulkanColorAttachment()                                      = default;
        FVulkanColorAttachment(const FVulkanColorAttachment&)           = delete;
        FVulkanColorAttachment& operator=(const FVulkanColorAttachment&) = delete;
    };

    FVulkanColorAttachment::
    FVulkanColorAttachment(FVulkanImageView* I_ImageView)
    : ImageView { I_ImageView }
    {
        VISERA_ASSERT(I_ImageView != nullptr);
    }

    vk::RenderingAttachmentInfo FVulkanColorAttachment::
    GetAttachmentInfo(vk::ImageLayout I_ImageLayout) const
    {
        vk::ClearValue ClearValue;
        ClearValue.color = GetClearColor();
        return vk::RenderingAttachmentInfo{}
            .setImageView   (ImageView->GetHandle())
            .setImageLayout (I_ImageLayout)
            .setLoadOp      (GetLoadOp())
            .setStoreOp     (GetStoreOp())
            .setClearValue  (ClearValue);
    }
}
