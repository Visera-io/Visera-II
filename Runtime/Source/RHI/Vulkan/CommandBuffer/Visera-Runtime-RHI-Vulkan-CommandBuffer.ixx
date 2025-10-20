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
        PushConstants(EVulkanShaderStage I_ShaderStages,
                      const void*        I_Data,
                      UInt32             I_Offset,
                      UInt32             I_Size);
        void inline
        Draw(UInt32 I_VertexCount, UInt32 I_InstanceCount,
             UInt32 I_FirstVertex, UInt32 I_FirstInstance) const;
        void inline
        LeaveRenderPass();
        void inline
        BlitImage(TSharedRef<FVulkanImage> I_SrcImage,
                  TSharedRef<FVulkanImage> I_DstImage);
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
        CurrentRenderPass = std::move(I_RenderPass);

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
        auto& Viewport = CurrentViewport.value();
        Handle.setViewport(0, Viewport);

        if (!CurrentScissor.has_value())
        {
            CurrentScissor = vk::Rect2D{}
                .setOffset({0,0})
                .setExtent({static_cast<UInt32>(Viewport.width),static_cast<UInt32>(Viewport.height)})
            ;
        }
        Handle.setScissor(0, CurrentScissor.value());

        Status = EStatus::InsideRenderPass;
    }

    void FVulkanCommandBuffer::
    PushConstants(EVulkanShaderStage I_ShaderStages,
                  const void*        I_Data,
                  UInt32             I_Offset,
                  UInt32             I_Size)
    {
        VISERA_ASSERT(IsInsideRenderPass());

        vkCmdPushConstants(*Handle,
            *CurrentRenderPass->GetPipelineLayout(),
            Int32(I_ShaderStages),
            I_Offset, I_Size, I_Data);
        // Handle.pushConstants(
        //     ,
        //     I_ShaderStages,
        //     I_Offset,
        //     Data);
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

        Handle.endRendering();

        CurrentViewport.reset();
        CurrentScissor.reset();

        CurrentRenderPass.reset();

        Status = EStatus::Recording;
    }

    void FVulkanCommandBuffer::
    BlitImage(TSharedRef<FVulkanImage> I_SrcImage,
              TSharedRef<FVulkanImage> I_DstImage)
    {
        VISERA_ASSERT(I_SrcImage->GetLayout() == EVulkanImageLayout::eTransferSrcOptimal);
        VISERA_ASSERT(I_DstImage->GetLayout() == EVulkanImageLayout::eTransferDstOptimal);
        //VISERA_ASSERT(!_SrcImage->EnabledMSAA() && !_DstImage->EnabledMSAA()); //vkCmdBlitImage must not be used for multisampled source or destination images. Use vkCmdResolveImage for this purpose.

        //[TODO]: As parameters
        constexpr auto Offset = vk::Offset3D{0, 0, 0};
        const auto&  SrcExtent = I_SrcImage->GetExtent();
        vk::Offset3D SrcRange(SrcExtent.width, SrcExtent.height, SrcExtent.depth);
        const auto&  DstExtent = I_DstImage->GetExtent();
        vk::Offset3D DstRange(DstExtent.width, DstExtent.height, DstExtent.depth);

        constexpr auto BlitSubresourceRange = vk::ImageSubresourceLayers{}
            .setAspectMask      (EVulkanImageAspect::eColor)
            .setMipLevel        (0)
            .setBaseArrayLayer  (0)
            .setLayerCount      (1)
        ;
        const auto BlitRegion = vk::ImageBlit2{}
            .setSrcSubresource (BlitSubresourceRange)
            .setSrcOffsets({Offset, SrcRange})
            .setDstSubresource (BlitSubresourceRange)
            .setDstOffsets     ({Offset, DstRange})
        ;
        const auto BlitInfo = vk::BlitImageInfo2{}
            .setSrcImage        (I_SrcImage->GetHandle())
            .setSrcImageLayout  (I_SrcImage->GetLayout())
            .setDstImage        (I_DstImage->GetHandle())
            .setDstImageLayout  (I_DstImage->GetLayout())
            .setRegionCount     (1)
            .setPRegions        (&BlitRegion)
            .setFilter          (vk::Filter::eLinear);
        ;
        Handle.blitImage2(BlitInfo);
    }

    void FVulkanCommandBuffer::
    End()
    {
        Handle.end();

        Status = EStatus::ReadyToSubmit;
    }
}
