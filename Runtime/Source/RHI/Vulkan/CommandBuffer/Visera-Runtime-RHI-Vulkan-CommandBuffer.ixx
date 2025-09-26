module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.CommandBuffer;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Interface.CommandBuffer;
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanCommandBuffer : public ICommandBuffer
    {
    public:
        void
        Begin() override;
        void
        ConvertImageLayout(TSharedPtr<ITexture2D> I_Texture,
                           EImageLayout           I_NewLayout,
                           EPipelineStage         I_SrcStage,
                           EAccess                I_SrcAccess,
                           EPipelineStage         I_DstStage,
                           EAccess                I_DstAccess) override;
        void
        EnterRenderPass(TSharedPtr<IRenderPass> I_RenderPass) override;
        void
        Draw(UInt32 I_VertexCount, UInt32 I_InstanceCount,
             UInt32 I_FirstVertex, UInt32 I_FirstInstance) const override;
        void
        End() override;
        void
        Submit(const void* I_Queue) override;

        const void*
        GetHandle() const override { return &Handle; }

    private:
        vk::raii::CommandBuffer Handle {nullptr};
        vk::Viewport   Viewport {};
        vk::Rect2D     Scissor  {};
        vk::ClearValue ClearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

    public:
        FVulkanCommandBuffer(const vk::raii::Device&      I_Device,
                             const vk::raii::CommandPool& I_Pool,
                             EType I_Type);
        ~FVulkanCommandBuffer() override {}
    };

    FVulkanCommandBuffer::
    FVulkanCommandBuffer(const vk::raii::Device&      I_Device,
                         const vk::raii::CommandPool& I_Pool,
                         EType I_Type)
    : ICommandBuffer{I_Type}
    {
        auto CreateInfo = vk::CommandBufferAllocateInfo{}
            .setCommandPool         (I_Pool)
            .setLevel               (vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount  (1)
        ;
        auto Results = I_Device.allocateCommandBuffers(CreateInfo);
        if (!Results)
        {  LOG_FATAL("Failed to create a new Vulkan Command Buffer (type:{})!", I_Type); }
        else
        { Handle = std::move(Results->front()); }
    }

    void FVulkanCommandBuffer::
    Begin()
    {
        VISERA_ASSERT(IsIdle() || IsSubmitted());

        Handle.begin({});

        Status = EStatus::Recording;
    }

    void FVulkanCommandBuffer::
    ConvertImageLayout(TSharedPtr<ITexture2D> I_Texture,
                       EImageLayout           I_NewLayout,
                       EPipelineStage         I_SrcStage,
                       EAccess                I_SrcAccess,
                       EPipelineStage         I_DstStage,
                       EAccess                I_DstAccess)
    {
        VISERA_ASSERT(IsRecording());

        auto OldLayout = AutoCast(I_Texture->GetLayout());
        auto NewLayout = AutoCast(I_NewLayout);

        if (OldLayout == NewLayout)
        {
            LOG_WARN("Skipped to convert a image to the same layout \"{}\"!",
                     I_NewLayout);
            return;
        }

        auto Image = static_cast<const vk::raii::Image*>(I_Texture->GetHandle());

        auto ImageSubresourceRange = vk::ImageSubresourceRange{}
            .setAspectMask      (vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel    (0)
            .setLevelCount      (1)
            .setBaseArrayLayer  (0)
            .setLayerCount      (1)
        ;
        auto Barrier = vk::ImageMemoryBarrier2{}
            .setSrcStageMask        (AutoCast(I_SrcStage))
            .setSrcAccessMask       (AutoCast(I_SrcAccess))
            .setDstStageMask        (AutoCast(I_DstStage))
            .setDstAccessMask       (AutoCast(I_DstAccess))
            .setOldLayout           (OldLayout)
            .setNewLayout           (NewLayout)
            .setSrcQueueFamilyIndex (vk::QueueFamilyIgnored)
            .setDstQueueFamilyIndex (vk::QueueFamilyIgnored)
            .setImage               (*Image)
            .setSubresourceRange    (ImageSubresourceRange)
        ;
        auto DependencyInfo = vk::DependencyInfo{}
            .setDependencyFlags     ({})
            .setImageMemoryBarrierCount(1)
            .setPImageMemoryBarriers(&Barrier)
        ;
        Handle.pipelineBarrier2(DependencyInfo);
    }

    void FVulkanCommandBuffer::
    EnterRenderPass(TSharedPtr<IRenderPass> I_RenderPass)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_RenderPass);

        const auto& RT = I_RenderPass->GetRenderTarget();
        auto RTView = static_cast<const vk::raii::ImageView*>(RT->GetView());
        auto AttachmentInfo = vk::RenderingAttachmentInfo{}
            .setImageView(*RTView)
        ;
        // {
        //     .imageView = swapChainImageViews[imageIndex],
        //     .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        //     .loadOp = vk::AttachmentLoadOp::eClear,
        //     .storeOp = vk::AttachmentStoreOp::eStore,
        //     .clearValue = clearColor
        // };

        auto Pipeline = static_cast<const vk::raii::Pipeline*>(I_RenderPass->GetPipeline());
        Handle.bindPipeline(vk::PipelineBindPoint::eGraphics,
                         *Pipeline);

        Status = EStatus::InsideRenderPass;
    }

    void FVulkanCommandBuffer::
    Draw(UInt32 I_VertexCount, UInt32 I_InstanceCount,
         UInt32 I_FirstVertex, UInt32 I_FirstInstance) const
    {
        VISERA_ASSERT(IsInsideRenderPass());
        Handle.draw(I_VertexCount,
            I_InstanceCount,
            I_FirstVertex,
            I_FirstInstance);
    }

    void FVulkanCommandBuffer::
    End()
    {
        VISERA_ASSERT(IsRecording());
        Handle.end();

        Status = EStatus::Recording;
    }

    void FVulkanCommandBuffer::
    Submit(const void* I_Queue)
    {
        VISERA_ASSERT(!IsSubmitted());


        Status = EStatus::Submitted;
    }
}
