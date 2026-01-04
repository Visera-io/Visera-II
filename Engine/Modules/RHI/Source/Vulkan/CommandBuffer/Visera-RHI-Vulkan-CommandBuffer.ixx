module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Vulkan.CommandBuffer;
#define VISERA_MODULE_NAME "RHI.Vulkan"
import Visera.RHI.Vulkan.Common;
import Visera.RHI.Vulkan.Pipeline;
import Visera.RHI.Vulkan.Image;
import Visera.RHI.Vulkan.Buffer;
import Visera.RHI.Vulkan.DescriptorSet;
import Visera.Runtime.Log;
import vulkan_hpp;

namespace Visera
{
    export class VISERA_RHI_API FVulkanCommandBuffer
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
        ConvertImageLayout(FVulkanImage*              I_Image,
                           vk::ImageLayout            I_NewLayout,
                           vk::PipelineStageFlagBits2 I_SrcStage,
                           vk::AccessFlagBits2        I_SrcAccess,
                           vk::PipelineStageFlagBits2 I_DstStage,
                           vk::AccessFlagBits2        I_DstAccess);
        void inline
        SetViewport(const vk::Viewport& I_Viewport) { CurrentViewport = I_Viewport; Handle.setViewport(0, CurrentViewport.value()); }
        void inline
        SetScissor(const vk::Rect2D& I_Scissor)     { CurrentScissor = I_Scissor;   Handle.setScissor(0, CurrentScissor.value());}
        void inline
        EnterRenderPipeline(FVulkanRenderPipeline* I_RenderPipeline);
        void inline
        BindVertexBuffer(UInt32                    I_Binding,
                         FVulkanBuffer*            I_VertexBuffer,
                         UInt64                    I_BufferOffset);
        void inline
        PushConstants(const void*          I_Data,
                      UInt32               I_Offset,
                      UInt32               I_Size);
        template<class T> void
        PushConstants(const T&             I_Data,
                      UInt32               I_Offset = 0) { PushConstants(&I_Data, I_Offset, sizeof(I_Data)); }
        void inline
        BindDescriptorSet(UInt32                           I_SetIndex,
                          FVulkanDescriptorSet*            I_DescriptorSet);
        void inline
        Draw(UInt32 I_VertexCount, UInt32 I_InstanceCount,
             UInt32 I_FirstVertex, UInt32 I_FirstInstance) const;
        void inline
        DrawIndexed(UInt32 I_IndexCount, UInt32 I_InstanceCount,
                    UInt32 I_FirstIndex, Int32  I_VertexOffset,
                    UInt32 I_FirstInstance) const;
        void inline
        LeaveRenderPipeline();
        void inline
        BlitImage(FVulkanImage* I_SrcImage,
                  FVulkanImage* I_DstImage);
        void inline
        CopyBufferToImage(FVulkanBuffer* I_SrcBuffer,
                          FVulkanImage*  I_DstImage);
        void inline
        EnterComputePipeline(FVulkanComputePipeline* I_ComputePipeline);
        void inline
        Dispatch(UInt32 I_GroupCountX, UInt32 I_GroupCountY, UInt32 I_GroupCountZ);
        void inline
        LeaveComputePipeline();
        void inline
        End();

        [[nodiscard]] inline const vk::raii::CommandBuffer&
        GetHandle() const { return Handle; }
        [[nodiscard]] inline EStatus
        GetStatus() const { return Status; }

        [[nodiscard]] inline Bool
        IsIdle() const { return Status == EStatus::Idle; }
        [[nodiscard]] inline Bool
        IsRecording() const { return Status == EStatus::Recording; }
        [[nodiscard]] inline Bool
        IsInsideRenderPass()  const { return Status == EStatus::InsideRenderPass; }
        [[nodiscard]] inline Bool
        IsReadyToSubmit() const { return Status == EStatus::ReadyToSubmit; }
        [[nodiscard]] inline Bool
        IsInsideComputePipeline() const { return CurrentComputePipeline != nullptr; }

    private:
        vk::raii::CommandBuffer            Handle {nullptr};
        TOptional<vk::Viewport>            CurrentViewport;
        TOptional<vk::Rect2D>              CurrentScissor;
        FVulkanRenderPipeline*             CurrentRenderPipeline {nullptr};
        FVulkanComputePipeline*            CurrentComputePipeline {nullptr};

        EStatus Status { EStatus::Idle };

    public:
        FVulkanCommandBuffer() = default;
        FVulkanCommandBuffer(const vk::raii::Device&      I_Device,
                             const vk::raii::CommandPool& I_Pool);
        FVulkanCommandBuffer(FVulkanCommandBuffer&&) = default;
        FVulkanCommandBuffer& operator=(FVulkanCommandBuffer&&) = default;
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
    ConvertImageLayout(FVulkanImage*              I_Image,
                       vk::ImageLayout            I_NewLayout,
                       vk::PipelineStageFlagBits2 I_SrcStage,
                       vk::AccessFlagBits2        I_SrcAccess,
                       vk::PipelineStageFlagBits2 I_DstStage,
                       vk::AccessFlagBits2        I_DstAccess)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_Image != nullptr);
        
        const auto ImageHandle = I_Image->GetHandle();

        auto OldLayout = I_Image->GetLayout();
        auto NewLayout = I_NewLayout;

        if (OldLayout == NewLayout)
        {
            LOG_WARN("Skipped to convert a image to the same layout \"{}\"!",
                     NewLayout);
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
            .setImage               (ImageHandle)
            .setSubresourceRange    (ImageSubresourceRange)
        ;
        I_Image->ConvertLayout(Handle, Barrier);
    }

    void FVulkanCommandBuffer::
    EnterRenderPipeline(FVulkanRenderPipeline* I_RenderPipeline)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_RenderPipeline != nullptr);
        CurrentRenderPipeline = I_RenderPipeline;

        auto& RenderingInfo = CurrentRenderPipeline->GetRenderingInfo();
        Handle.beginRendering(RenderingInfo);

        Handle.bindPipeline(vk::PipelineBindPoint::eGraphics,
                            CurrentRenderPipeline->GetHandle());

        if (!CurrentViewport.has_value())
        {
            SetViewport(vk::Viewport{}
                .setX       (RenderingInfo.renderArea.offset.x)
                .setY       (RenderingInfo.renderArea.offset.y)
                .setWidth   (RenderingInfo.renderArea.extent.width)
                .setHeight  (RenderingInfo.renderArea.extent.height)
                .setMinDepth(0.0)
                .setMaxDepth(1.0));
        }

        if (!CurrentScissor.has_value())
        {
            SetScissor(vk::Rect2D{}
                .setOffset(RenderingInfo.renderArea.offset)
                .setExtent(RenderingInfo.renderArea.extent));
        }

        Status = EStatus::InsideRenderPass;
    }

    void FVulkanCommandBuffer::
    PushConstants(const void*          I_Data,
                  UInt32               I_Offset,
                  UInt32               I_Size)
    {
        VISERA_ASSERT(IsInsideRenderPass() || IsInsideComputePipeline());
        VISERA_ASSERT((I_Offset % 4) == 0); // VUID Constrain
        VISERA_ASSERT((I_Size   % 4) == 0); // VUID Constrain

        FVulkanPipelineLayout* PipelineLayout = nullptr;
        vk::ShaderStageFlags StageFlags;
        if (CurrentRenderPipeline)
        {
            PipelineLayout = CurrentRenderPipeline->GetLayout();
            StageFlags = PipelineLayout->GetPushConstantStages();
        }
        else if (CurrentComputePipeline)
        {
            PipelineLayout = CurrentComputePipeline->GetLayout();
            StageFlags = vk::ShaderStageFlagBits::eCompute;
        }
        else
        {
            VISERA_ASSERT(False && "No pipeline bound!");
            return;
        }

        const auto Info = vk::PushConstantsInfo{}
            .setLayout      (PipelineLayout->GetHandle())
            .setStageFlags  (StageFlags)
            .setOffset      (I_Offset)
            .setSize        (I_Size)
            .setPValues     (I_Data)
        ;
        Handle.pushConstants2(Info);
    }

    void FVulkanCommandBuffer::
    BindVertexBuffer(UInt32                    I_Binding,
                     FVulkanBuffer*            I_VertexBuffer,
                     UInt64                    I_BufferOffset)
    {
        VISERA_ASSERT(IsInsideRenderPass());
        VISERA_ASSERT(I_VertexBuffer != nullptr);
        Handle.bindVertexBuffers2(
            I_Binding,
            {I_VertexBuffer->GetHandle()},
            {I_BufferOffset}
        );
    }

    void FVulkanCommandBuffer::
    BindDescriptorSet(UInt32                           I_SetIndex,
                      FVulkanDescriptorSet*            I_DescriptorSet)
    {
        VISERA_ASSERT(IsInsideRenderPass() || IsInsideComputePipeline());
        VISERA_ASSERT(I_DescriptorSet != nullptr);
        FVulkanPipelineLayout* PipelineLayout = nullptr;
        vk::ShaderStageFlags StageFlags;
        if (CurrentRenderPipeline)
        {
            PipelineLayout = CurrentRenderPipeline->GetLayout();
            StageFlags = PipelineLayout->GetDescriptorSetStages();
        }
        else if (CurrentComputePipeline)
        {
            PipelineLayout = CurrentComputePipeline->GetLayout();
            StageFlags = vk::ShaderStageFlagBits::eCompute;
        }
        else
        {
            VISERA_ASSERT(False && "No pipeline bound!");
            return;
        }

        const auto DescriptorSet = I_DescriptorSet->GetHandle();

        const auto BindInfo = vk::BindDescriptorSetsInfo{}
            .setLayout              (PipelineLayout->GetHandle())
            .setStageFlags          (StageFlags)
            .setFirstSet            (I_SetIndex)
            .setDescriptorSetCount  (1)
            .setPDescriptorSets     (&DescriptorSet)
        ;
        Handle.bindDescriptorSets2(BindInfo);
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
    DrawIndexed(UInt32 I_IndexCount, UInt32 I_InstanceCount,
                UInt32 I_FirstIndex, Int32  I_VertexOffset,
                UInt32 I_FirstInstance) const
    {
        VISERA_ASSERT(IsInsideRenderPass());
        Handle.drawIndexed(I_IndexCount,
            I_InstanceCount,
            I_FirstIndex,
            I_VertexOffset,
            I_FirstInstance);
    }

    void FVulkanCommandBuffer::
    LeaveRenderPipeline()
    {
        VISERA_ASSERT(IsInsideRenderPass());

        Handle.endRendering();

        CurrentViewport.reset();
        CurrentScissor.reset();

        CurrentRenderPipeline = nullptr;

        Status = EStatus::Recording;
    }

    void FVulkanCommandBuffer::
    EnterComputePipeline(FVulkanComputePipeline* I_ComputePipeline)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_ComputePipeline != nullptr);
        VISERA_ASSERT(CurrentRenderPipeline == nullptr); // Cannot be in render pass
        CurrentComputePipeline = I_ComputePipeline;

        Handle.bindPipeline(vk::PipelineBindPoint::eCompute,
                            CurrentComputePipeline->GetHandle());
    }

    void FVulkanCommandBuffer::
    LeaveComputePipeline()
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(CurrentComputePipeline != nullptr);

        CurrentComputePipeline = nullptr;
    }

    void FVulkanCommandBuffer::
    Dispatch(UInt32 I_GroupCountX, UInt32 I_GroupCountY, UInt32 I_GroupCountZ)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(CurrentComputePipeline != nullptr);

        Handle.dispatch(I_GroupCountX, I_GroupCountY, I_GroupCountZ);
    }

    void FVulkanCommandBuffer::
    BlitImage(FVulkanImage* I_SrcImage,
              FVulkanImage* I_DstImage)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_SrcImage != nullptr);
        VISERA_ASSERT(I_DstImage != nullptr);
        VISERA_ASSERT(I_SrcImage->GetLayout() == vk::ImageLayout::eTransferSrcOptimal);
        VISERA_ASSERT(I_DstImage->GetLayout() == vk::ImageLayout::eTransferDstOptimal);
        //VISERA_ASSERT(!_SrcImage->EnabledMSAA() && !_DstImage->EnabledMSAA()); //vkCmdBlitImage must not be used for multisampled source or destination images. Use vkCmdResolveImage for this purpose.

        //[TODO]: As parameters
        constexpr auto Offset = vk::Offset3D{0, 0, 0};
        const auto&  SrcExtent = I_SrcImage->GetExtent();
        vk::Offset3D SrcRange(SrcExtent.width, SrcExtent.height, SrcExtent.depth);
        const auto&  DstExtent = I_DstImage->GetExtent();
        vk::Offset3D DstRange(DstExtent.width, DstExtent.height, DstExtent.depth);

        constexpr auto BlitSubresourceRange = vk::ImageSubresourceLayers{}
            .setAspectMask      (vk::ImageAspectFlagBits::eColor)
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
    CopyBufferToImage(FVulkanBuffer* I_SrcBuffer,
                      FVulkanImage*  I_DstImage)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_SrcBuffer != nullptr);
        VISERA_ASSERT(I_DstImage != nullptr);
        VISERA_ASSERT(I_DstImage->GetLayout() == vk::ImageLayout::eTransferDstOptimal);
        VISERA_ASSERT(I_SrcBuffer->GetMemorySize() <= I_DstImage->GetMemorySize());
        constexpr auto ImageSubresourceRange = vk::ImageSubresourceLayers{}
            .setAspectMask      (vk::ImageAspectFlagBits::eColor)
            .setMipLevel        (0)
            .setBaseArrayLayer  (0)
            .setLayerCount      (1)
        ;
        auto CopyRegion = vk::BufferImageCopy2{}
            .setBufferOffset        (0)
            .setBufferRowLength     (0) // 0 => tightly packed, row pitch = width * bytesPerPixel
            .setBufferImageHeight   (0) // 0 => tightly packed
            .setImageSubresource    (ImageSubresourceRange)
            .setImageOffset         ({0, 0, 0})
            .setImageExtent         (I_DstImage->GetExtent())
        ;
        auto CopyInfo = vk::CopyBufferToImageInfo2{}
            .setSrcBuffer(I_SrcBuffer->GetHandle())
            .setDstImage(I_DstImage->GetHandle())
            .setDstImageLayout(I_DstImage->GetLayout())
            .setRegionCount(1)
            .setPRegions(&CopyRegion)
        ;
        Handle.copyBufferToImage2(CopyInfo);
    }

    void FVulkanCommandBuffer::
    End()
    {
        VISERA_ASSERT(IsRecording());
        Handle.end();

        Status = EStatus::ReadyToSubmit;
    }
}
