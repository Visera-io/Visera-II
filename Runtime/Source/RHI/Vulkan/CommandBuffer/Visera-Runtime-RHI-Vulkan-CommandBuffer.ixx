module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.CommandBuffer;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Runtime.RHI.Vulkan.RenderPass;
import Visera.Runtime.RHI.Vulkan.Texture2D;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanCommandBuffer
    {
    public:
        enum class EStatus : UInt8
        {
            Idle,
            Recording,
            InsideRenderPass,
            Submitted,
        };

        void inline
        Begin();
        void inline
        ConvertImageLayout(TSharedRef<FVulkanTexture2D> I_Texture,
                           EVulkanImageLayout           I_NewLayout,
                           EVulkanPipelineStage         I_SrcStage,
                           EVulkanAccess                I_SrcAccess,
                           EVulkanPipelineStage         I_DstStage,
                           EVulkanAccess                I_DstAccess);
        void inline
        EnterRenderPass(TSharedRef<FVulkanRenderPass> I_RenderPass);
        void inline
        Draw(UInt32 I_VertexCount, UInt32 I_InstanceCount,
             UInt32 I_FirstVertex, UInt32 I_FirstInstance) const;
        void inline
        LeaveRenderPass() { CurrentRenderPass.reset(); }
        void inline
        End();
        void inline
        Submit(const void* I_Queue);

        [[nodiscard]] inline const vk::raii::CommandBuffer&
        GetHandle() const { return Handle; }

        [[nodiscard]] inline Bool
        IsIdle() const { return Status == EStatus::Idle; }
        [[nodiscard]] inline Bool
        IsRecording() const { return Status == EStatus::Recording; }
        [[nodiscard]] inline Bool
        IsInsideRenderPass() const { return Status == EStatus::InsideRenderPass; }
        [[nodiscard]] inline Bool
        IsSubmitted() const { return Status == EStatus::Submitted; }

    private:
        vk::raii::CommandBuffer Handle {nullptr};
        vk::Viewport   Viewport {};
        vk::Rect2D     Scissor  {};
        vk::ClearValue ClearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
        TSharedPtr<FVulkanRenderPass> CurrentRenderPass;

        EStatus Status { EStatus::Idle };

    public:
        FVulkanCommandBuffer(const vk::raii::Device&      I_Device,
                             const vk::raii::CommandPool& I_Pool);
        ~FVulkanCommandBuffer() {}
    };

    FVulkanCommandBuffer::
    FVulkanCommandBuffer(const vk::raii::Device&      I_Device,
                         const vk::raii::CommandPool& I_Pool)
    {
        auto CreateInfo = vk::CommandBufferAllocateInfo{}
            .setCommandPool         (I_Pool)
            .setLevel               (vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount  (1)
        ;
        auto Results = I_Device.allocateCommandBuffers(CreateInfo);
        if (!Results)
        {  LOG_FATAL("Failed to create a new Vulkan Command Buffer!"); }
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
    ConvertImageLayout(TSharedRef<FVulkanTexture2D> I_Texture,
                       EVulkanImageLayout           I_NewLayout,
                       EVulkanPipelineStage         I_SrcStage,
                       EVulkanAccess                I_SrcAccess,
                       EVulkanPipelineStage         I_DstStage,
                       EVulkanAccess                I_DstAccess)
    {
        VISERA_ASSERT(IsRecording());

        auto OldLayout = I_Texture->GetLayout();
        auto NewLayout = I_NewLayout;

        if (OldLayout == NewLayout)
        {
            LOG_WARN("Skipped to convert a image to the same layout \"{}\"!",
                     I_NewLayout);
            return;
        }

        auto ImageSubresourceRange = vk::ImageSubresourceRange{}
            .setAspectMask      (vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel    (0)
            .setLevelCount      (1)
            .setBaseArrayLayer  (0)
            .setLayerCount      (1)
        ;
        auto Barrier = vk::ImageMemoryBarrier2{}
            .setSrcStageMask        (I_SrcStage)
            .setSrcAccessMask       (I_SrcAccess)
            .setDstStageMask        (I_DstStage)
            .setDstAccessMask       (I_DstAccess)
            .setOldLayout           (OldLayout)
            .setNewLayout           (NewLayout)
            .setSrcQueueFamilyIndex (vk::QueueFamilyIgnored)
            .setDstQueueFamilyIndex (vk::QueueFamilyIgnored)
            .setImage               (I_Texture->GetHandle())
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
    EnterRenderPass(TSharedRef<FVulkanRenderPass> I_RenderPass)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_RenderPass);

        const auto& RT = I_RenderPass->GetRenderTarget();
        auto AttachmentInfo = vk::RenderingAttachmentInfo{}
            .setImageView(RT->GetView())
        ;
        // {
        //     .imageView = swapChainImageViews[imageIndex],
        //     .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        //     .loadOp = vk::AttachmentLoadOp::eClear,
        //     .storeOp = vk::AttachmentStoreOp::eStore,
        //     .clearValue = clearColor
        // };

        Handle.bindPipeline(vk::PipelineBindPoint::eGraphics,
                         I_RenderPass->GetPipeline());

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
