module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Vulkan.CommandBuffer;
#define VISERA_MODULE_NAME "RHI.Vulkan"
import Visera.RHI.Vulkan.Common;
import Visera.RHI.Vulkan.Pipeline;
import Visera.RHI.Vulkan.Image;
import Visera.RHI.Vulkan.Buffer;
import Visera.RHI.Vulkan.DescriptorSet;
import Visera.Global.Log;
import vulkan_hpp;

export namespace Visera
{
    template<EVulkanQueueFamily QueueFamily>
    class VISERA_RHI_API FVulkanCommandBuffer
    {
    public:
        enum class EStatus : UInt8
        {
            Idle,
            Recording,
            ReadyToSubmit,    // ===Queue::Submit==> Idle (by Reset())
        };

        void
        Reset();
        void
        Begin();
        void
        End();

        [[nodiscard]] inline  vk::CommandBuffer
        GetHandle() const { return Handle; }
        [[nodiscard]] inline EStatus
        GetStatus() const { return Status; }

        [[nodiscard]] inline Bool
        IsIdle() const { return Status == EStatus::Idle; }
        [[nodiscard]] inline Bool
        IsRecording() const { return Status == EStatus::Recording; }
        [[nodiscard]] inline Bool
        IsReadyToSubmit() const { return Status == EStatus::ReadyToSubmit; }

    protected:
        vk::CommandBuffer           Handle {nullptr};
        EStatus                     Status { EStatus::Idle };

    public:
        FVulkanCommandBuffer() = default;
        FVulkanCommandBuffer(const vk::raii::CommandPool&         I_CommandPool,
                             const vk::CommandBufferAllocateInfo& I_CreateInfo);
        FVulkanCommandBuffer(FVulkanCommandBuffer&&) = default;
        FVulkanCommandBuffer& operator=(FVulkanCommandBuffer&&) = default;
        ~FVulkanCommandBuffer() {}
    };

    template<>
    class VISERA_RHI_API FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>
    {
    public:
        static constexpr EVulkanQueueFamily
        QueueFamily = EVulkanQueueFamily::Transfer;

        enum class EStatus : UInt8
        {
            Idle,
            Recording,
            ReadyToSubmit,    // ===Queue::Submit==> Idle (by Reset())
        };

        void
        Reset();
        void
        Begin();
        void
        ConvertImageLayout(FVulkanImage*           I_Image,
                           vk::ImageLayout         I_NewLayout,
                           EVulkanTransferStage    I_SrcStage,
                           vk::AccessFlags2        I_SrcAccess,
                           EVulkanTransferStage    I_DstStage,
                           vk::AccessFlags2        I_DstAccess);
        void
        BlitImage(FVulkanImage* I_SrcImage,
                  FVulkanImage* I_DstImage);
        void
        CopyBufferToImage(FVulkanBuffer* I_SrcBuffer,
                          FVulkanImage*  I_DstImage);
        void
        End();

        [[nodiscard]] inline  vk::CommandBuffer
        GetHandle() const { return Handle; }
        [[nodiscard]] inline EStatus
        GetStatus() const { return Status; }

        [[nodiscard]] inline Bool
        IsIdle() const { return Status == EStatus::Idle; }
        [[nodiscard]] inline Bool
        IsRecording() const { return Status == EStatus::Recording; }
        [[nodiscard]] inline Bool
        IsReadyToSubmit() const { return Status == EStatus::ReadyToSubmit; }

    private:
        vk::CommandBuffer           Handle {nullptr};
        EStatus                     Status { EStatus::Idle };

    public:
        FVulkanCommandBuffer() = default;
        FVulkanCommandBuffer(const vk::raii::CommandPool&         I_CommandPool,
                             const vk::CommandBufferAllocateInfo& I_CreateInfo);
        FVulkanCommandBuffer(FVulkanCommandBuffer&&) = default;
        FVulkanCommandBuffer& operator=(FVulkanCommandBuffer&&) = default;
        ~FVulkanCommandBuffer() {}
    };

    template<>
    class VISERA_RHI_API FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>
    {
    public:
        static constexpr EVulkanQueueFamily
        QueueFamily = EVulkanQueueFamily::Graphics;

        enum class EStatus : UInt8
        {
            Idle,
            Recording,
            InsideRenderPass,
            ReadyToSubmit,    // ===Queue::Submit==> Idle (by Reset())
        };

        void
        Reset();
        void
        Begin();
        void
        ConvertImageLayout(FVulkanImage*           I_Image,
                           vk::ImageLayout         I_NewLayout,
                           EVulkanGraphicsStage    I_SrcStage,
                           vk::AccessFlags2        I_SrcAccess,
                           EVulkanGraphicsStage    I_DstStage,
                           vk::AccessFlags2        I_DstAccess);
        void
        SetViewport(const vk::Viewport& I_Viewport) { CurrentViewport = I_Viewport; Handle.setViewport(0, CurrentViewport.value()); }
        void
        SetScissor(const vk::Rect2D& I_Scissor)     { CurrentScissor = I_Scissor;   Handle.setScissor(0, CurrentScissor.value());}
        void
        EnterRenderPipeline(FVulkanRenderPipeline* I_RenderPipeline);
        void
        BindVertexBuffer(UInt32                    I_Binding,
                         FVulkanBuffer*            I_VertexBuffer,
                         UInt64                    I_BufferOffset);
        void
        PushConstants(const void*          I_Data,
                      UInt32               I_Offset,
                      UInt32               I_Size);
        template<class T> void
        PushConstants(const T&             I_Data,
                      UInt32               I_Offset = 0) { PushConstants(&I_Data, I_Offset, sizeof(I_Data)); }
        void
        BindDescriptorSet(UInt32                           I_SetIndex,
                          FVulkanDescriptorSet*            I_DescriptorSet);
        void
        Draw(UInt32 I_VertexCount, UInt32 I_InstanceCount,
             UInt32 I_FirstVertex, UInt32 I_FirstInstance) const;
        void
        DrawIndexed(UInt32 I_IndexCount, UInt32 I_InstanceCount,
                    UInt32 I_FirstIndex, Int32  I_VertexOffset,
                    UInt32 I_FirstInstance) const;
        void
        LeaveRenderPipeline();
        void
        BlitImage(FVulkanImage* I_SrcImage,
                  FVulkanImage* I_DstImage);
        void
        CopyBufferToImage(FVulkanBuffer* I_SrcBuffer,
                          FVulkanImage*  I_DstImage);
        void
        End();

        [[nodiscard]] inline  vk::CommandBuffer
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

    private:
        vk::CommandBuffer           Handle {nullptr};
        TOptional<vk::Viewport>     CurrentViewport;
        TOptional<vk::Rect2D>       CurrentScissor;
        FVulkanRenderPipeline*      CurrentRenderPipeline  {nullptr};

        EStatus Status { EStatus::Idle };

    public:
        FVulkanCommandBuffer() = default;
        FVulkanCommandBuffer(const vk::raii::CommandPool&         I_CommandPool,
                             const vk::CommandBufferAllocateInfo& I_CreateInfo);
        FVulkanCommandBuffer(FVulkanCommandBuffer&&) = default;
        FVulkanCommandBuffer& operator=(FVulkanCommandBuffer&&) = default;
        ~FVulkanCommandBuffer() {}
    };

    template<>
    class VISERA_RHI_API FVulkanCommandBuffer<EVulkanQueueFamily::Compute>
    {
    public:
        static constexpr EVulkanQueueFamily
        QueueFamily = EVulkanQueueFamily::Compute;

        enum class EStatus : UInt8
        {
            Idle,
            Recording,
            ReadyToSubmit,    // ===Queue::Submit==> Idle (by Reset())
        };

        void
        Reset();
        void
        Begin();
        void
        ConvertImageLayout(FVulkanImage*           I_Image,
                           vk::ImageLayout         I_NewLayout,
                           EVulkanComputeStage     I_SrcStage,
                           vk::AccessFlags2        I_SrcAccess,
                           EVulkanComputeStage     I_DstStage,
                           vk::AccessFlags2        I_DstAccess);
        void
        PushConstants(const void*          I_Data,
                      UInt32               I_Offset,
                      UInt32               I_Size);
        template<class T> void
        PushConstants(const T&             I_Data,
                      UInt32               I_Offset = 0) { PushConstants(&I_Data, I_Offset, sizeof(I_Data)); }
        void
        BindDescriptorSet(UInt32                           I_SetIndex,
                          FVulkanDescriptorSet*            I_DescriptorSet);
        void
        EnterComputePipeline(FVulkanComputePipeline* I_ComputePipeline);
        void
        Dispatch(UInt32 I_GroupCountX, UInt32 I_GroupCountY, UInt32 I_GroupCountZ);
        void
        LeaveComputePipeline();
        void
        BlitImage(FVulkanImage* I_SrcImage,
                  FVulkanImage* I_DstImage);
        void
        CopyBufferToImage(FVulkanBuffer* I_SrcBuffer,
                          FVulkanImage*  I_DstImage);
        void
        End();

        [[nodiscard]] inline  vk::CommandBuffer
        GetHandle() const { return Handle; }
        [[nodiscard]] inline EStatus
        GetStatus() const { return Status; }

        [[nodiscard]] inline Bool
        IsIdle() const { return Status == EStatus::Idle; }
        [[nodiscard]] inline Bool
        IsRecording() const { return Status == EStatus::Recording; }
        [[nodiscard]] inline Bool
        IsReadyToSubmit() const { return Status == EStatus::ReadyToSubmit; }
        [[nodiscard]] inline Bool
        IsInsideComputePipeline() const { return CurrentComputePipeline != nullptr; }

    private:
        vk::CommandBuffer           Handle {nullptr};
        FVulkanComputePipeline*     CurrentComputePipeline {nullptr};

        EStatus Status { EStatus::Idle };

    public:
        FVulkanCommandBuffer() = default;
        FVulkanCommandBuffer(const vk::raii::CommandPool&         I_CommandPool,
                             const vk::CommandBufferAllocateInfo& I_CreateInfo);
        FVulkanCommandBuffer(FVulkanCommandBuffer&&) = default;
        FVulkanCommandBuffer& operator=(FVulkanCommandBuffer&&) = default;
        ~FVulkanCommandBuffer() {}
    };

    namespace Concepts
    {
        template<class T> concept
        CommandBuffer = requires(T I_Cmd)
        {
            { T::QueueFamily            } -> std::convertible_to<EVulkanQueueFamily>;
            { I_Cmd.GetHandle()         } -> std::same_as<vk::CommandBuffer>;
            { I_Cmd.IsReadyToSubmit()   } -> std::convertible_to<Bool>;
        };
    }

    template<EVulkanQueueFamily QueueFamily>
    FVulkanCommandBuffer<QueueFamily>::
    FVulkanCommandBuffer(const vk::raii::CommandPool&         I_CommandPool,
                         const vk::CommandBufferAllocateInfo& I_CreateInfo)
    {
        auto Results = I_CommandPool.getDevice().allocateCommandBuffers(I_CreateInfo);
        if (!Results.has_value())
        {  LOG_FATAL("Failed to create a new Vulkan Command Buffer!"); }
        else
        { Handle = std::move(Results->front()); }
    }

    template<EVulkanQueueFamily QueueFamily>
    void FVulkanCommandBuffer<QueueFamily>::
    Reset()
    {
        VISERA_ASSERT(IsReadyToSubmit() || IsIdle());

        Handle.reset();

        Status = EStatus::Idle;
    }

    template<EVulkanQueueFamily QueueFamily>
    void FVulkanCommandBuffer<QueueFamily>::
    Begin()
    {
        VISERA_ASSERT(IsIdle());

        auto BeginInfo = vk::CommandBufferBeginInfo{}
        ;
        auto Result = Handle.begin(BeginInfo);

        Status = EStatus::Recording;
    }

    template<EVulkanQueueFamily QueueFamily>
    void FVulkanCommandBuffer<QueueFamily>::
    End()
    {
        VISERA_ASSERT(IsRecording());
        Handle.end();

        Status = EStatus::ReadyToSubmit;
    }

    FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
    FVulkanCommandBuffer(const vk::raii::CommandPool&         I_CommandPool,
                         const vk::CommandBufferAllocateInfo& I_CreateInfo)
    {
        auto Results = I_CommandPool.getDevice().allocateCommandBuffers(I_CreateInfo);
        if (!Results.has_value())
        {  LOG_FATAL("Failed to create a new Vulkan Command Buffer!"); }
        else
        { Handle = std::move(Results->front()); }
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>::
    Reset()
    {
        VISERA_ASSERT(IsReadyToSubmit() || IsIdle());

        Handle.reset();

        Status = EStatus::Idle;
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>::
    Begin()
    {
        VISERA_ASSERT(IsIdle());

        auto BeginInfo = vk::CommandBufferBeginInfo{}
        ;
        auto Result = Handle.begin(BeginInfo);

        Status = EStatus::Recording;
    }

    FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>::
    FVulkanCommandBuffer(const vk::raii::CommandPool&         I_CommandPool,
                         const vk::CommandBufferAllocateInfo& I_CreateInfo)
    {
        auto Results = I_CommandPool.getDevice().allocateCommandBuffers(I_CreateInfo);
        if (!Results.has_value())
        {  LOG_FATAL("Failed to create a new Vulkan Command Buffer!"); }
        else
        { Handle = std::move(Results->front()); }
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>::
    ConvertImageLayout(FVulkanImage*           I_Image,
                       vk::ImageLayout         I_NewLayout,
                       EVulkanTransferStage    I_SrcStage,
                       vk::AccessFlags2        I_SrcAccess,
                       EVulkanTransferStage    I_DstStage,
                       vk::AccessFlags2        I_DstAccess)
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
            .setLevelCount      (I_Image->GetMipmapLevels())
            .setBaseArrayLayer  (0)
            .setLayerCount      (I_Image->GetArrayLayers())
        ;
        auto Barrier = vk::ImageMemoryBarrier2{}
            .setSrcStageMask        (TypeCast(I_SrcStage))
            .setSrcAccessMask       (I_SrcAccess)
            .setDstStageMask        (TypeCast(I_DstStage))
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

    void FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>::
    BlitImage(FVulkanImage* I_SrcImage,
              FVulkanImage* I_DstImage)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_SrcImage != nullptr);
        VISERA_ASSERT(I_DstImage != nullptr);
        VISERA_ASSERT(I_SrcImage->GetLayout() == vk::ImageLayout::eTransferSrcOptimal);
        VISERA_ASSERT(I_DstImage->GetLayout() == vk::ImageLayout::eTransferDstOptimal);

        const auto Offset = vk::Offset3D{0, 0, 0};
        const auto&  SrcExtent = I_SrcImage->GetExtent();
        vk::Offset3D SrcRange(SrcExtent.width, SrcExtent.height, SrcExtent.depth);
        const auto&  DstExtent = I_DstImage->GetExtent();
        vk::Offset3D DstRange(DstExtent.width, DstExtent.height, DstExtent.depth);

        const auto BlitSubresourceRange = vk::ImageSubresourceLayers{}
            .setAspectMask      (vk::ImageAspectFlagBits::eColor)
            .setMipLevel        (I_SrcImage->GetMipmapLevels())
            .setBaseArrayLayer  (0)
            .setLayerCount      (I_SrcImage->GetArrayLayers())
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
            .setFilter          (vk::Filter::eLinear)
        ;
        Handle.blitImage2(BlitInfo);
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>::
    CopyBufferToImage(FVulkanBuffer* I_SrcBuffer,
                      FVulkanImage*  I_DstImage)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_SrcBuffer != nullptr);
        VISERA_ASSERT(I_DstImage != nullptr);
        VISERA_ASSERT(I_DstImage->GetLayout() == vk::ImageLayout::eTransferDstOptimal);
        VISERA_ASSERT(I_SrcBuffer->GetMemorySize() <= I_DstImage->GetMemorySize());
        const auto ImageSubresourceRange = vk::ImageSubresourceLayers{}
            .setAspectMask      (vk::ImageAspectFlagBits::eColor)
            .setMipLevel        (I_DstImage->GetMipmapLevels())
            .setBaseArrayLayer  (0)
            .setLayerCount      (I_DstImage->GetArrayLayers())
        ;
        auto CopyRegion = vk::BufferImageCopy2{}
            .setBufferOffset        (0)
            .setBufferRowLength     (0)
            .setBufferImageHeight   (0)
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

    void FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>::
    End()
    {
        VISERA_ASSERT(IsRecording());
        Handle.end();

        Status = EStatus::ReadyToSubmit;
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
    Reset()
    {
        VISERA_ASSERT(IsReadyToSubmit() || IsIdle());

        Handle.reset();

        Status = EStatus::Idle;
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
    Begin()
    {
        VISERA_ASSERT(IsIdle());

        auto BeginInfo = vk::CommandBufferBeginInfo{}
        ;
        auto Result = Handle.begin(BeginInfo);

        Status = EStatus::Recording;
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
    ConvertImageLayout(FVulkanImage*           I_Image,
                       vk::ImageLayout         I_NewLayout,
                       EVulkanGraphicsStage    I_SrcStage,
                       vk::AccessFlags2        I_SrcAccess,
                       EVulkanGraphicsStage    I_DstStage,
                       vk::AccessFlags2        I_DstAccess)
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
            .setLevelCount      (I_Image->GetMipmapLevels())
            .setBaseArrayLayer  (0)
            .setLayerCount      (I_Image->GetArrayLayers())
        ;
        auto Barrier = vk::ImageMemoryBarrier2{}
            .setSrcStageMask        (TypeCast(I_SrcStage))
            .setSrcAccessMask       (I_SrcAccess)
            .setDstStageMask        (TypeCast(I_DstStage))
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
    
    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
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

    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
    PushConstants(const void*          I_Data,
                  UInt32               I_Offset,
                  UInt32               I_Size)
    {
        VISERA_ASSERT(IsInsideRenderPass());
        VISERA_ASSERT((I_Offset % 4) == 0);
        VISERA_ASSERT((I_Size   % 4) == 0);
        VISERA_ASSERT(CurrentRenderPipeline != nullptr);

        auto* PipelineLayout = CurrentRenderPipeline->GetLayout();
        auto StageFlags = PipelineLayout->GetPushConstantStages();

        const auto Info = vk::PushConstantsInfo{}
            .setLayout      (PipelineLayout->GetHandle())
            .setStageFlags  (StageFlags)
            .setOffset      (I_Offset)
            .setSize        (I_Size)
            .setPValues     (I_Data)
        ;
        Handle.pushConstants2(Info);
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
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

    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
    BindDescriptorSet(UInt32                           I_SetIndex,
                      FVulkanDescriptorSet*            I_DescriptorSet)
    {
        VISERA_ASSERT(IsInsideRenderPass());
        VISERA_ASSERT(I_DescriptorSet != nullptr);
        VISERA_ASSERT(CurrentRenderPipeline != nullptr);

        auto* PipelineLayout = CurrentRenderPipeline->GetLayout();
        auto StageFlags = PipelineLayout->GetDescriptorSetStages();

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

    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
    Draw(UInt32 I_VertexCount, UInt32 I_InstanceCount,
         UInt32 I_FirstVertex, UInt32 I_FirstInstance) const
    {
        VISERA_ASSERT(IsInsideRenderPass());
        Handle.draw(I_VertexCount,
            I_InstanceCount,
            I_FirstVertex,
            I_FirstInstance);
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
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

    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
    LeaveRenderPipeline()
    {
        VISERA_ASSERT(IsInsideRenderPass());

        Handle.endRendering();

        CurrentViewport.reset();
        CurrentScissor.reset();

        CurrentRenderPipeline = nullptr;

        Status = EStatus::Recording;
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
    BlitImage(FVulkanImage* I_SrcImage,
              FVulkanImage* I_DstImage)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_SrcImage != nullptr);
        VISERA_ASSERT(I_DstImage != nullptr);
        VISERA_ASSERT(I_SrcImage->GetLayout() == vk::ImageLayout::eTransferSrcOptimal);
        VISERA_ASSERT(I_DstImage->GetLayout() == vk::ImageLayout::eTransferDstOptimal);

        const auto Offset = vk::Offset3D{0, 0, 0};
        const auto&  SrcExtent = I_SrcImage->GetExtent();
        vk::Offset3D SrcRange(SrcExtent.width, SrcExtent.height, SrcExtent.depth);
        const auto&  DstExtent = I_DstImage->GetExtent();
        vk::Offset3D DstRange(DstExtent.width, DstExtent.height, DstExtent.depth);

        const auto BlitSubresourceRange = vk::ImageSubresourceLayers{}
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
            .setFilter          (vk::Filter::eLinear)
        ;
        Handle.blitImage2(BlitInfo);
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
    CopyBufferToImage(FVulkanBuffer* I_SrcBuffer,
                      FVulkanImage*  I_DstImage)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_SrcBuffer != nullptr);
        VISERA_ASSERT(I_DstImage != nullptr);
        VISERA_ASSERT(I_DstImage->GetLayout() == vk::ImageLayout::eTransferDstOptimal);
        VISERA_ASSERT(I_SrcBuffer->GetMemorySize() <= I_DstImage->GetMemorySize());
        const auto ImageSubresourceRange = vk::ImageSubresourceLayers{}
            .setAspectMask      (vk::ImageAspectFlagBits::eColor)
            .setMipLevel        (0)
            .setBaseArrayLayer  (0)
            .setLayerCount      (1)
        ;
        auto CopyRegion = vk::BufferImageCopy2{}
            .setBufferOffset        (0)
            .setBufferRowLength     (0)
            .setBufferImageHeight   (0)
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

    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
    End()
    {
        VISERA_ASSERT(IsRecording());
        Handle.end();

        Status = EStatus::ReadyToSubmit;
    }

    FVulkanCommandBuffer<EVulkanQueueFamily::Compute>::
    FVulkanCommandBuffer(const vk::raii::CommandPool&         I_CommandPool,
                         const vk::CommandBufferAllocateInfo& I_CreateInfo)
    {
        auto Results = I_CommandPool.getDevice().allocateCommandBuffers(I_CreateInfo);
        if (!Results.has_value())
        {  LOG_FATAL("Failed to create a new Vulkan Command Buffer!"); }
        else
        { Handle = std::move(Results->front()); }
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Compute>::
    Reset()
    {
        VISERA_ASSERT(IsReadyToSubmit() || IsIdle());

        Handle.reset();

        Status = EStatus::Idle;
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Compute>::
    Begin()
    {
        VISERA_ASSERT(IsIdle());

        auto BeginInfo = vk::CommandBufferBeginInfo{}
        ;
        auto Result = Handle.begin(BeginInfo);

        Status = EStatus::Recording;
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Compute>::
    ConvertImageLayout(FVulkanImage*           I_Image,
                       vk::ImageLayout         I_NewLayout,
                       EVulkanComputeStage     I_SrcStage,
                       vk::AccessFlags2        I_SrcAccess,
                       EVulkanComputeStage     I_DstStage,
                       vk::AccessFlags2        I_DstAccess)
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
            .setLevelCount      (I_Image->GetMipmapLevels())
            .setBaseArrayLayer  (0)
            .setLayerCount      (I_Image->GetArrayLayers())
        ;
        auto Barrier = vk::ImageMemoryBarrier2{}
            .setSrcStageMask        (TypeCast(I_SrcStage))
            .setSrcAccessMask       (I_SrcAccess)
            .setDstStageMask        (TypeCast(I_DstStage))
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

    void FVulkanCommandBuffer<EVulkanQueueFamily::Compute>::
    PushConstants(const void*          I_Data,
                  UInt32               I_Offset,
                  UInt32               I_Size)
    {
        VISERA_ASSERT(IsInsideComputePipeline());
        VISERA_ASSERT((I_Offset % 4) == 0);
        VISERA_ASSERT((I_Size   % 4) == 0);
        VISERA_ASSERT(CurrentComputePipeline != nullptr);

        auto* PipelineLayout = CurrentComputePipeline->GetLayout();
        auto StageFlags = vk::ShaderStageFlagBits::eCompute;

        const auto Info = vk::PushConstantsInfo{}
            .setLayout      (PipelineLayout->GetHandle())
            .setStageFlags  (StageFlags)
            .setOffset      (I_Offset)
            .setSize        (I_Size)
            .setPValues     (I_Data)
        ;
        Handle.pushConstants2(Info);
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Compute>::
    BindDescriptorSet(UInt32                           I_SetIndex,
                      FVulkanDescriptorSet*            I_DescriptorSet)
    {
        VISERA_ASSERT(IsInsideComputePipeline());
        VISERA_ASSERT(I_DescriptorSet != nullptr);
        VISERA_ASSERT(CurrentComputePipeline != nullptr);

        auto* PipelineLayout = CurrentComputePipeline->GetLayout();
        auto StageFlags = vk::ShaderStageFlagBits::eCompute;

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

    void FVulkanCommandBuffer<EVulkanQueueFamily::Compute>::
    EnterComputePipeline(FVulkanComputePipeline* I_ComputePipeline)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_ComputePipeline != nullptr);
        CurrentComputePipeline = I_ComputePipeline;

        Handle.bindPipeline(vk::PipelineBindPoint::eCompute,
                            CurrentComputePipeline->GetHandle());
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Compute>::
    LeaveComputePipeline()
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(CurrentComputePipeline != nullptr);

        CurrentComputePipeline = nullptr;
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Compute>::
    Dispatch(UInt32 I_GroupCountX, UInt32 I_GroupCountY, UInt32 I_GroupCountZ)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(CurrentComputePipeline != nullptr);

        Handle.dispatch(I_GroupCountX, I_GroupCountY, I_GroupCountZ);
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Compute>::
    CopyBufferToImage(FVulkanBuffer* I_SrcBuffer,
                      FVulkanImage*  I_DstImage)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_SrcBuffer != nullptr);
        VISERA_ASSERT(I_DstImage != nullptr);
        VISERA_ASSERT(I_DstImage->GetLayout() == vk::ImageLayout::eTransferDstOptimal);
        VISERA_ASSERT(I_SrcBuffer->GetMemorySize() <= I_DstImage->GetMemorySize());
        const auto ImageSubresourceRange = vk::ImageSubresourceLayers{}
            .setAspectMask      (vk::ImageAspectFlagBits::eColor)
            .setMipLevel        (0)
            .setBaseArrayLayer  (0)
            .setLayerCount      (1)
        ;
        auto CopyRegion = vk::BufferImageCopy2{}
            .setBufferOffset        (0)
            .setBufferRowLength     (0)
            .setBufferImageHeight   (0)
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

    void FVulkanCommandBuffer<EVulkanQueueFamily::Compute>::
    End()
    {
        VISERA_ASSERT(IsRecording());
        Handle.end();

        Status = EStatus::ReadyToSubmit;
    }
}
