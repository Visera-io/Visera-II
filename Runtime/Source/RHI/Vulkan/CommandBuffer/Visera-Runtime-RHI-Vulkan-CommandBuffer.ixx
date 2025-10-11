module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.CommandBuffer;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Runtime.RHI.Vulkan.RenderPass;
import Visera.Runtime.RHI.Vulkan.Image;
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
            ReadyToSubmit,    // ===Queue::Submit==> Idle (by Reset())
        };

        void inline
        Reset();
        void inline
        Begin();
        void inline
        ConvertImageLayout(TSharedRef<FVulkanImage> I_Image,
                           EVulkanImageLayout       I_NewLayout,
                           EVulkanPipelineStage     I_SrcStage,
                           EVulkanAccess            I_SrcAccess,
                           EVulkanPipelineStage     I_DstStage,
                           EVulkanAccess            I_DstAccess);
        void inline
        EnterRenderPass(TSharedRef<FVulkanRenderPass> I_RenderPass);
        void inline
        Draw(UInt32 I_VertexCount, UInt32 I_InstanceCount,
             UInt32 I_FirstVertex, UInt32 I_FirstInstance) const;
        void inline
        LeaveRenderPass();
        void inline
        End();

        [[nodiscard]] inline const vk::raii::CommandBuffer&
        GetHandle() const { return Handle; }

        [[nodiscard]] inline Bool
        IsIdle() const { return Status == EStatus::Idle; }
        [[nodiscard]] inline Bool
        IsRecording() const { return Status == EStatus::Recording; }
        [[nodiscard]] inline Bool
        IsInsideRenderPass() const { return Status == EStatus::InsideRenderPass; }
        [[nodiscard]] inline Bool
        IsReadyToSubmit() const { return Status == EStatus::ReadyToSubmit; }

    private:
        vk::raii::CommandBuffer       Handle {nullptr};
        TOptional<FVulkanViewport>    CurrentViewport;
        TOptional<FVulkanRect2D>      CurrentScissor;
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
        if (!Results.has_value())
        {  LOG_FATAL("Failed to create a new Vulkan Command Buffer!"); }
        else
        { Handle = std::move(Results->front()); }
    }

    void FVulkanCommandBuffer::
    Reset()
    {
        VISERA_ASSERT(IsReadyToSubmit() || IsIdle());

        Handle.reset();

        Status = EStatus::Idle;
    }

    void FVulkanCommandBuffer::
    Begin()
    {
        VISERA_ASSERT(IsIdle()); // Forgot to Reset()?

        Handle.begin({});

        Status = EStatus::Recording;
    }

    void FVulkanCommandBuffer::
    ConvertImageLayout(TSharedRef<FVulkanImage> I_Image,
                       EVulkanImageLayout       I_NewLayout,
                       EVulkanPipelineStage     I_SrcStage,
                       EVulkanAccess            I_SrcAccess,
                       EVulkanPipelineStage     I_DstStage,
                       EVulkanAccess            I_DstAccess)
    {
        VISERA_ASSERT(IsRecording());

        auto OldLayout = I_Image->GetLayout();
        auto NewLayout = I_NewLayout;

        if (OldLayout == NewLayout)
        {
            LOG_WARN("Skipped to convert a image to the same layout \"{}\"!",
                     NewLayout);
            return;
        }

        auto ImageSubresourceRange = vk::ImageSubresourceRange{}
            .setAspectMask      (EVulkanImageAspect::eColor)
            .setBaseMipLevel    (0)
            .setLevelCount      (1)
            .setBaseArrayLayer  (0)
            .setLayerCount      (1)
        ;
        auto Barrier = FVulkanImageBarrier{}
            .setSrcStageMask        (I_SrcStage)
            .setSrcAccessMask       (I_SrcAccess)
            .setDstStageMask        (I_DstStage)
            .setDstAccessMask       (I_DstAccess)
            .setOldLayout           (OldLayout)
            .setNewLayout           (NewLayout)
            .setSrcQueueFamilyIndex (vk::QueueFamilyIgnored)
            .setDstQueueFamilyIndex (vk::QueueFamilyIgnored)
            .setImage               (I_Image->GetHandle())
            .setSubresourceRange    (ImageSubresourceRange)
        ;
        I_Image->ConvertLayout(Handle, Barrier);
    }

    void FVulkanCommandBuffer::
    EnterRenderPass(TSharedRef<FVulkanRenderPass> I_RenderPass)
    {
        VISERA_ASSERT(IsRecording());
        CurrentRenderPass = I_RenderPass;

        auto RenderingInfo = CurrentRenderPass->GetRenderingInfo();
        Handle.beginRendering(RenderingInfo);

        Handle.bindPipeline(vk::PipelineBindPoint::eGraphics,
                         CurrentRenderPass->GetPipeline());

        if (!CurrentViewport.has_value())
        {
            CurrentViewport = FVulkanViewport{}
                .setX       (RenderingInfo.renderArea.offset.x)
                .setY       (RenderingInfo.renderArea.offset.y)
                .setWidth   (RenderingInfo.renderArea.extent.width)
                .setHeight  (RenderingInfo.renderArea.extent.height)
            ;
        }
        Handle.setViewport(0, CurrentViewport.value());

        if (CurrentScissor.has_value())
        { Handle.setScissor(0, CurrentScissor.value()); }

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
    LeaveRenderPass()
    {
        VISERA_ASSERT(IsInsideRenderPass());

        CurrentViewport.reset();
        CurrentScissor.reset();

        CurrentRenderPass.reset();

        Status = EStatus::Recording;
    }

    void FVulkanCommandBuffer::
    End()
    {
        Handle.end();

        Status = EStatus::ReadyToSubmit;
    }
}
