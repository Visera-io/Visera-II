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
        [[nodiscard]] inline const vk::raii::ImageView&
        GetView() const  { return View; }
        [[nodiscard]] inline EVulkanImageLayout
        GetLayout() const  { return Layout; }
        [[nodiscard]] inline const vk::raii::Image&
        GetHandle() const { return Handle; }
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

    private:
        vk::raii::Image     Handle {nullptr};
        vk::raii::ImageView View   {nullptr};
        EVulkanImageLayout  Layout { EVulkanImageLayout::eUndefined };

        EVulkanLoadOp       LoadOp     { EVulkanLoadOp::eNone  };
        EVulkanStoreOp      StoreOp    { EVulkanStoreOp::eNone };
        FVulkanClearColor   ClearColor { FVulkanClearColor(0.0f, 0.0f, 0.0f, 1.0f)} ;

    public:
        FVulkanRenderTarget()                                      = default;
        FVulkanRenderTarget(int k);
        ~FVulkanRenderTarget()                                     = default;
        FVulkanRenderTarget(const FVulkanRenderTarget&)            = delete;
        FVulkanRenderTarget& operator=(const FVulkanRenderTarget&) = delete;
    };

    FVulkanRenderTarget::
    FVulkanRenderTarget(int k)
    {

    }

    vk::RenderingAttachmentInfo FVulkanRenderTarget::
    GetAttachmentInfo() const
    {
        auto AttachmentInfo = vk::RenderingAttachmentInfo{}
            .setImageView   (GetView())
            .setImageLayout (GetLayout())
            .setLoadOp      (GetLoadOp())
            .setStoreOp     (GetStoreOp())
            .setClearValue  (GetClearColor())
        ;
        return AttachmentInfo;
    }
}
