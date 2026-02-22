module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.Vulkan.CommandBuffer;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.Types.Optional;
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Runtime.RHI.Vulkan.Pipeline;
import Visera.Runtime.RHI.Vulkan.Image;
import Visera.Runtime.RHI.Vulkan.Buffer;
import Visera.Runtime.RHI.Vulkan.DescriptorSet;
import Visera.Core.Log;
import vulkan_hpp;

export namespace Visera
{
    // --- Status traits ---
    struct FCommandStatusBasic
    {
        enum class E : UInt8 { Idle, Recording, ReadyToSubmit };
    };

    struct FCommandStatusGraphics
    {
        enum class E : UInt8 { Idle, Recording, InsideRenderPass, ReadyToSubmit };
    };

    template<EVulkanQueueFamily Q>
    struct TVulkanCommandTraits
    {
        static_assert(static_cast<int>(Q) != static_cast<int>(Q),
            "TVulkanCommandTraits is not specialized for this QueueFamily.");
    };

    template<>
    struct TVulkanCommandTraits<EVulkanQueueFamily::Transfer>
    {
        using Status = FCommandStatusBasic::E;
    };

    template<>
    struct TVulkanCommandTraits<EVulkanQueueFamily::Compute>
    {
        using Status = FCommandStatusBasic::E;
    };

    template<>
    struct TVulkanCommandTraits<EVulkanQueueFamily::Graphics>
    {
        using Status = FCommandStatusGraphics::E;
    };

    // Opaque primary template; only Transfer/Compute/Graphics specializations are defined.
    template<EVulkanQueueFamily Q>
    class VISERA_RUNTIME_API FVulkanCommandBuffer;

    // --- Base (non-template) ---
    class VISERA_RUNTIME_API FVulkanCommandBufferBase
    {
    protected:
        vk::CommandBuffer Handle{nullptr};

        FVulkanCommandBufferBase() = default;

        static vk::CommandBuffer
        Allocate(const vk::raii::CommandPool& I_Pool,
                 const vk::CommandBufferAllocateInfo& I_Info)
        {
            auto Results = I_Pool.getDevice().allocateCommandBuffers(I_Info);
            if (!Results.has_value())
            { LOG_FATAL("Failed to create a new Vulkan Command Buffer!"); }
            return std::move(Results->front());
        }
    };

    // --- Common template (Reset/Begin/End/Handle/Status) ---
    template<EVulkanQueueFamily Q>
    class VISERA_RUNTIME_API TVulkanCommandBufferCommon : public FVulkanCommandBufferBase
    {
    public:
        static constexpr EVulkanQueueFamily QueueFamily = Q;
        using EStatus = typename TVulkanCommandTraits<Q>::Status;

        void Reset();
        void Begin(vk::CommandBufferUsageFlags I_Flags = {});
        void End();

        [[nodiscard]] inline vk::CommandBuffer
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
        EStatus Status{EStatus::Idle};

    public:
        TVulkanCommandBufferCommon() = default;
        TVulkanCommandBufferCommon(const vk::raii::CommandPool& I_CommandPool,
                                   const vk::CommandBufferAllocateInfo& I_CreateInfo)
        {
            Handle = Allocate(I_CommandPool, I_CreateInfo);
        }
        TVulkanCommandBufferCommon(TVulkanCommandBufferCommon&&) = default;
        TVulkanCommandBufferCommon& operator=(TVulkanCommandBufferCommon&&) = default;
        ~TVulkanCommandBufferCommon() = default;
    };

    // --- Transfer specialization ---
    template<>
    class VISERA_RUNTIME_API FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>
        : public TVulkanCommandBufferCommon<EVulkanQueueFamily::Transfer>
    {
        using Super = TVulkanCommandBufferCommon<EVulkanQueueFamily::Transfer>;

    public:
        using Super::Super;

        void ConvertImageLayout(FVulkanImage*           I_Image,
                               vk::ImageLayout         I_OldLayout,
                               vk::ImageLayout         I_NewLayout,
                               EVulkanTransferStage    I_SrcStage,
                               EVulkanTransferAccess   I_SrcAccess,
                               EVulkanTransferStage    I_DstStage,
                               EVulkanTransferAccess   I_DstAccess);
        void BlitImage(FVulkanImage*     I_SrcImage,
                      FVulkanImage*     I_DstImage,
                      vk::Filter        I_Filter,
                      vk::ImageLayout   I_SrcImageLayout,
                      vk::ImageLayout   I_DstImageLayout);
        void CopyBuffer(FVulkanBuffer* I_SrcBuffer,
                       FVulkanBuffer* I_DstBuffer,
                       UInt64         I_SrcOffset = 0,
                       UInt64         I_DstOffset = 0,
                       UInt64         I_Size = 0);
        void CopyBufferToImage(FVulkanBuffer* I_SrcBuffer,
                              FVulkanImage*  I_DstImage,
                              vk::ImageLayout I_DstImageLayout = vk::ImageLayout::eTransferDstOptimal);
        void CopyBufferToImage(FVulkanBuffer* I_SrcBuffer,
                              FVulkanImage*  I_DstImage,
                              UInt64         I_BufferOffset,
                              vk::ImageLayout I_DstImageLayout = vk::ImageLayout::eTransferDstOptimal);
    };

    // --- Compute specialization ---
    template<>
    class VISERA_RUNTIME_API FVulkanCommandBuffer<EVulkanQueueFamily::Compute>
        : public TVulkanCommandBufferCommon<EVulkanQueueFamily::Compute>
    {
        using Super = TVulkanCommandBufferCommon<EVulkanQueueFamily::Compute>;

    private:
        FVulkanComputePipeline* CurrentComputePipeline{nullptr};

    public:
        using Super::Super;

        void ConvertImageLayout(FVulkanImage*           I_Image,
                               vk::ImageLayout         I_OldLayout,
                               vk::ImageLayout         I_NewLayout,
                               EVulkanComputeStage     I_SrcStage,
                               EVulkanComputeAccess    I_SrcAccess,
                               EVulkanComputeStage     I_DstStage,
                               EVulkanComputeAccess    I_DstAccess);
        void PushConstants(const void* I_Data,
                          UInt32      I_Offset,
                          UInt32      I_Size);
        template<class T> void
        PushConstants(const T& I_Data, UInt32 I_Offset = 0)
        {
            PushConstants(&I_Data, I_Offset, sizeof(I_Data));
        }
        void BindDescriptorSet(UInt32                I_SetIndex,
                              FVulkanDescriptorSet* I_DescriptorSet);
        void EnterComputePipeline(FVulkanComputePipeline* I_ComputePipeline);
        void Dispatch(UInt32 I_GroupCountX, UInt32 I_GroupCountY, UInt32 I_GroupCountZ);
        void LeaveComputePipeline();
        void CopyBufferToImage(FVulkanBuffer*  I_SrcBuffer,
                              FVulkanImage*   I_DstImage,
                              vk::ImageLayout I_DstImageLayout = vk::ImageLayout::eTransferDstOptimal);

        [[nodiscard]] inline Bool
        IsInsideComputePipeline() const { return CurrentComputePipeline != nullptr; }
    };

    // --- Graphics specialization ---
    template<>
    class VISERA_RUNTIME_API FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>
        : public TVulkanCommandBufferCommon<EVulkanQueueFamily::Graphics>
    {
        using Super = TVulkanCommandBufferCommon<EVulkanQueueFamily::Graphics>;

    private:
        TOptional<vk::Viewport>     CurrentViewport;
        TOptional<vk::Rect2D>      CurrentScissor;
        FVulkanRenderPipeline*     CurrentRenderPipeline{nullptr};

    public:
        using Super::Super;

        void ConvertImageLayout(FVulkanImage*           I_Image,
                               vk::ImageLayout         I_OldLayout,
                               vk::ImageLayout         I_NewLayout,
                               EVulkanGraphicsStage    I_SrcStage,
                               EVulkanGraphicsAccess   I_SrcAccess,
                               EVulkanGraphicsStage    I_DstStage,
                               EVulkanGraphicsAccess   I_DstAccess);
        void ClearColorImage(FVulkanImage* I_Image, const vk::ClearColorValue& I_ClearColor,
                            vk::ImageLayout I_ImageLayout);
        void SetViewport(const vk::Viewport& I_Viewport);
        void SetScissor(const vk::Rect2D& I_Scissor);
        void EnterRenderPipeline(FVulkanRenderPipeline* I_RenderPipeline);
        void BindVertexBuffer(UInt32         I_Binding,
                             FVulkanBuffer* I_VertexBuffer,
                             UInt64         I_BufferOffset);
        void PushConstants(const void* I_Data,
                          UInt32      I_Offset,
                          UInt32      I_Size);
        template<class T> void
        PushConstants(const T& I_Data, UInt32 I_Offset = 0)
        {
            PushConstants(&I_Data, I_Offset, sizeof(I_Data));
        }
        void BindDescriptorSet(UInt32                I_SetIndex,
                              FVulkanDescriptorSet* I_DescriptorSet);
        void Draw(UInt32 I_VertexCount, UInt32 I_InstanceCount,
                 UInt32 I_FirstVertex, UInt32 I_FirstInstance) const;
        void DrawIndexed(UInt32 I_IndexCount, UInt32 I_InstanceCount,
                        UInt32 I_FirstIndex, Int32  I_VertexOffset,
                        UInt32 I_FirstInstance) const;
        void LeaveRenderPipeline();
        void BlitImage(FVulkanImage*     I_SrcImage,
                      FVulkanImage*     I_DstImage,
                      vk::Filter        I_Filter,
                      vk::ImageLayout   I_SrcImageLayout,
                      vk::ImageLayout   I_DstImageLayout);
        void CopyBufferToImage(FVulkanBuffer*  I_SrcBuffer,
                              FVulkanImage*   I_DstImage,
                              vk::ImageLayout I_DstImageLayout = vk::ImageLayout::eTransferDstOptimal);

        [[nodiscard]] inline Bool
        IsInsideRenderPass() const { return Status == EStatus::InsideRenderPass; }

        void Reset()
        {
            VISERA_ASSERT(!IsInsideRenderPass());
            Super::Reset();
        }

        void End()
        {
            VISERA_ASSERT(!IsInsideRenderPass());
            Super::End();
        }
    };

    namespace Concepts
    {
        template<class T> concept
        CommandBuffer = requires(T I_Command)
        {
            { T::QueueFamily          } -> std::convertible_to<EVulkanQueueFamily>;
            { I_Command.GetHandle()       } -> std::same_as<vk::CommandBuffer>;
            { I_Command.IsReadyToSubmit() } -> std::convertible_to<Bool>;
        };
    }

    // ========== TVulkanCommandBufferCommon implementations ==========
    template<EVulkanQueueFamily Q>
    void TVulkanCommandBufferCommon<Q>::Reset()
    {
        VISERA_ASSERT(IsReadyToSubmit() || IsIdle());
        (void)Handle.reset();
        Status = EStatus::Idle;
    }

    template<EVulkanQueueFamily Q>
    void TVulkanCommandBufferCommon<Q>::Begin(vk::CommandBufferUsageFlags I_Flags)
    {
        VISERA_ASSERT(IsIdle());
        auto BeginInfo = vk::CommandBufferBeginInfo{};
        BeginInfo.flags = I_Flags;
        (void)Handle.begin(BeginInfo);
        Status = EStatus::Recording;
    }

    template<EVulkanQueueFamily Q>
    void TVulkanCommandBufferCommon<Q>::End()
    {
        VISERA_ASSERT(IsRecording());
        (void)Handle.end();
        Status = EStatus::ReadyToSubmit;
    }

    // ========== Transfer implementations ==========
    void FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>::
    ConvertImageLayout(FVulkanImage*           I_Image,
                       vk::ImageLayout         I_OldLayout,
                       vk::ImageLayout         I_NewLayout,
                       EVulkanTransferStage    I_SrcStage,
                       EVulkanTransferAccess   I_SrcAccess,
                       EVulkanTransferStage    I_DstStage,
                       EVulkanTransferAccess   I_DstAccess)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_Image != nullptr);

        auto OldLayout = I_OldLayout;
        auto NewLayout = I_NewLayout;
        if (NewLayout == vk::ImageLayout::eUndefined || OldLayout == NewLayout)
        { return; }

        const auto ImageHandle = I_Image->GetHandle();
        auto ImageSubresourceRange = vk::ImageSubresourceRange{}
            .setAspectMask    (vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel  (0)
            .setLevelCount    (I_Image->GetMipmapLevels())
            .setBaseArrayLayer(0)
            .setLayerCount    (I_Image->GetArrayLayers());

        auto Barrier = vk::ImageMemoryBarrier2{}
            .setSrcStageMask       (TypeCast(I_SrcStage))
            .setSrcAccessMask      (TypeCast(I_SrcAccess))
            .setDstStageMask       (TypeCast(I_DstStage))
            .setDstAccessMask      (TypeCast(I_DstAccess))
            .setOldLayout          (OldLayout)
            .setNewLayout          (NewLayout)
            .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setImage              (ImageHandle)
            .setSubresourceRange   (ImageSubresourceRange);
        I_Image->ConvertLayout(Handle, Barrier);
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>::
    BlitImage(FVulkanImage*     I_SrcImage,
              FVulkanImage*     I_DstImage,
              vk::Filter        I_Filter,
              vk::ImageLayout   I_SrcImageLayout,
              vk::ImageLayout   I_DstImageLayout)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_SrcImage != nullptr);
        VISERA_ASSERT(I_DstImage != nullptr);

        const auto Offset = vk::Offset3D{0, 0, 0};
        const auto& SrcExtent = I_SrcImage->GetExtent();
        vk::Offset3D SrcRange(SrcExtent.width, SrcExtent.height, SrcExtent.depth);
        const auto& DstExtent = I_DstImage->GetExtent();
        vk::Offset3D DstRange(DstExtent.width, DstExtent.height, DstExtent.depth);

        const auto BlitRegion = vk::ImageBlit2{}
            .setSrcSubresource (I_SrcImage->GetSubresourceLayers(0))
            .setSrcOffsets({Offset, SrcRange})
            .setDstSubresource (I_DstImage->GetSubresourceLayers(0))
            .setDstOffsets     ({Offset, DstRange});
        const auto BlitInfo = vk::BlitImageInfo2{}
            .setSrcImage       (I_SrcImage->GetHandle())
            .setSrcImageLayout (I_SrcImageLayout)
            .setDstImage       (I_DstImage->GetHandle())
            .setDstImageLayout (I_DstImageLayout)
            .setRegionCount    (1)
            .setPRegions       (&BlitRegion)
            .setFilter        (I_Filter);
        Handle.blitImage2(BlitInfo);
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>::
    CopyBuffer(FVulkanBuffer* I_SrcBuffer,
               FVulkanBuffer* I_DstBuffer,
               UInt64         I_SrcOffset,
               UInt64         I_DstOffset,
               UInt64         I_Size)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_SrcBuffer != nullptr);
        VISERA_ASSERT(I_DstBuffer != nullptr);

        const UInt64 CopySize = (I_Size > 0)
            ? I_Size
            : (std::min)(I_SrcBuffer->GetMemorySize() - I_SrcOffset,
                         I_DstBuffer->GetMemorySize() - I_DstOffset);
        VISERA_ASSERT(CopySize > 0);

        const auto CopyRegion = vk::BufferCopy2{}
            .setSrcOffset(I_SrcOffset)
            .setDstOffset(I_DstOffset)
            .setSize    (CopySize);

        const auto CopyInfo = vk::CopyBufferInfo2{}
            .setSrcBuffer   (I_SrcBuffer->GetHandle())
            .setDstBuffer   (I_DstBuffer->GetHandle())
            .setRegionCount (1)
            .setPRegions    (&CopyRegion);

        Handle.copyBuffer2(CopyInfo);
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>::
    CopyBufferToImage(FVulkanBuffer* I_SrcBuffer,
                      FVulkanImage*  I_DstImage,
                      vk::ImageLayout I_DstImageLayout)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_SrcBuffer != nullptr);
        VISERA_ASSERT(I_DstImage  != nullptr);
        VISERA_ASSERT(I_SrcBuffer->GetMemorySize() <= I_DstImage->GetMemorySize());

        auto CopyRegion = vk::BufferImageCopy2{}
            .setBufferOffset      (0)
            .setBufferRowLength   (0)
            .setBufferImageHeight (0)
            .setImageSubresource  (I_DstImage->GetSubresourceLayers(0))
            .setImageOffset       ({0, 0, 0})
            .setImageExtent       (I_DstImage->GetExtent());
        auto CopyInfo = vk::CopyBufferToImageInfo2{}
            .setSrcBuffer   (I_SrcBuffer->GetHandle())
            .setDstImage   (I_DstImage->GetHandle())
            .setDstImageLayout(I_DstImageLayout)
            .setRegionCount (1)
            .setPRegions    (&CopyRegion);
        Handle.copyBufferToImage2(CopyInfo);
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>::
    CopyBufferToImage(FVulkanBuffer* I_SrcBuffer,
                      FVulkanImage*  I_DstImage,
                      UInt64         I_BufferOffset,
                      vk::ImageLayout I_DstImageLayout)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_SrcBuffer != nullptr);
        VISERA_ASSERT(I_DstImage  != nullptr);

        auto CopyRegion = vk::BufferImageCopy2{}
            .setBufferOffset      (I_BufferOffset)
            .setBufferRowLength   (0)
            .setBufferImageHeight (0)
            .setImageSubresource  (I_DstImage->GetSubresourceLayers(0))
            .setImageOffset       ({0, 0, 0})
            .setImageExtent       (I_DstImage->GetExtent());
        auto CopyInfo = vk::CopyBufferToImageInfo2{}
            .setSrcBuffer      (I_SrcBuffer->GetHandle())
            .setDstImage       (I_DstImage->GetHandle())
            .setDstImageLayout (I_DstImageLayout)
            .setRegionCount    (1)
            .setPRegions       (&CopyRegion);
        Handle.copyBufferToImage2(CopyInfo);
    }

    // ========== Graphics implementations ==========
    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
    SetViewport(const vk::Viewport& I_Viewport)
    {
        CurrentViewport = I_Viewport;
        Handle.setViewport(0, 1, &CurrentViewport.GetValue());
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
    SetScissor(const vk::Rect2D& I_Scissor)
    {
        CurrentScissor = I_Scissor;
        Handle.setScissor(0, 1, &CurrentScissor.GetValue());
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
    ConvertImageLayout(FVulkanImage*           I_Image,
                       vk::ImageLayout         I_OldLayout,
                       vk::ImageLayout         I_NewLayout,
                       EVulkanGraphicsStage    I_SrcStage,
                       EVulkanGraphicsAccess   I_SrcAccess,
                       EVulkanGraphicsStage    I_DstStage,
                       EVulkanGraphicsAccess   I_DstAccess)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_Image != nullptr);

        auto OldLayout = I_OldLayout;
        auto NewLayout = I_NewLayout;
        if (NewLayout == vk::ImageLayout::eUndefined || OldLayout == NewLayout)
        { return; }

        const auto ImageHandle = I_Image->GetHandle();
        auto ImageSubresourceRange = vk::ImageSubresourceRange{}
            .setAspectMask    (vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel  (0)
            .setLevelCount    (I_Image->GetMipmapLevels())
            .setBaseArrayLayer(0)
            .setLayerCount    (I_Image->GetArrayLayers());

        auto Barrier = vk::ImageMemoryBarrier2{}
            .setSrcStageMask       (TypeCast(I_SrcStage))
            .setSrcAccessMask      (TypeCast(I_SrcAccess))
            .setDstStageMask       (TypeCast(I_DstStage))
            .setDstAccessMask      (TypeCast(I_DstAccess))
            .setOldLayout          (OldLayout)
            .setNewLayout          (NewLayout)
            .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setImage              (ImageHandle)
            .setSubresourceRange   (ImageSubresourceRange);
        I_Image->ConvertLayout(Handle, Barrier);
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
    ClearColorImage(FVulkanImage* I_Image, const vk::ClearColorValue& I_ClearColor,
                    vk::ImageLayout I_ImageLayout)
    {
        auto ResourceRange = I_Image->GetSubresourceRange();
        Handle.clearColorImage(I_Image->GetHandle(),
            I_ImageLayout,
            &I_ClearColor,
            1,
            &ResourceRange);
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

        if (!CurrentViewport.HasValue())
        {
            SetViewport(vk::Viewport{}
                .setX       (RenderingInfo.renderArea.offset.x)
                .setY       (RenderingInfo.renderArea.offset.y)
                .setWidth   (RenderingInfo.renderArea.extent.width)
                .setHeight  (RenderingInfo.renderArea.extent.height)
                .setMinDepth(0.0)
                .setMaxDepth(1.0));
        }

        if (!CurrentScissor.HasValue())
        {
            SetScissor(vk::Rect2D{}
                .setOffset(RenderingInfo.renderArea.offset)
                .setExtent(RenderingInfo.renderArea.extent));
        }

        Status = EStatus::InsideRenderPass;
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
    PushConstants(const void* I_Data,
                  UInt32      I_Offset,
                  UInt32      I_Size)
    {
        VISERA_ASSERT(IsInsideRenderPass());
        VISERA_ASSERT((I_Offset % 4) == 0);
        VISERA_ASSERT((I_Size   % 4) == 0);
        VISERA_ASSERT(CurrentRenderPipeline != nullptr);

        auto* PipelineLayout = CurrentRenderPipeline->GetLayout();
        auto StageFlags = PipelineLayout->GetPushConstantStages();

        const auto Info = vk::PushConstantsInfo{}
            .setLayout     (PipelineLayout->GetHandle())
            .setStageFlags (StageFlags)
            .setOffset     (I_Offset)
            .setSize       (I_Size)
            .setPValues    (I_Data);
        Handle.pushConstants2(Info);
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
    BindVertexBuffer(UInt32         I_Binding,
                     FVulkanBuffer* I_VertexBuffer,
                     UInt64         I_BufferOffset)
    {
        VISERA_ASSERT(IsInsideRenderPass());
        VISERA_ASSERT(I_VertexBuffer != nullptr);
        Handle.bindVertexBuffers2(
            I_Binding,
            {I_VertexBuffer->GetHandle()},
            {I_BufferOffset});
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
    BindDescriptorSet(UInt32                I_SetIndex,
                      FVulkanDescriptorSet* I_DescriptorSet)
    {
        VISERA_ASSERT(IsInsideRenderPass());
        VISERA_ASSERT(I_DescriptorSet != nullptr);
        VISERA_ASSERT(CurrentRenderPipeline != nullptr);

        auto* PipelineLayout = CurrentRenderPipeline->GetLayout();
        auto StageFlags = PipelineLayout->GetDescriptorSetStages();

        const auto DescriptorSet = I_DescriptorSet->GetHandle();

        const auto BindInfo = vk::BindDescriptorSetsInfo{}
            .setLayout             (PipelineLayout->GetHandle())
            .setStageFlags         (StageFlags)
            .setFirstSet           (I_SetIndex)
            .setDescriptorSetCount (1)
            .setPDescriptorSets    (&DescriptorSet);
        Handle.bindDescriptorSets2(BindInfo);
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
    Draw(UInt32 I_VertexCount, UInt32 I_InstanceCount,
         UInt32 I_FirstVertex, UInt32 I_FirstInstance) const
    {
        VISERA_ASSERT(Status == EStatus::InsideRenderPass);
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
        VISERA_ASSERT(Status == EStatus::InsideRenderPass);
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

        CurrentViewport.Reset();
        CurrentScissor.Reset();

        CurrentRenderPipeline = nullptr;

        Status = EStatus::Recording;
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
    BlitImage(FVulkanImage*     I_SrcImage,
              FVulkanImage*     I_DstImage,
              vk::Filter        I_Filter,
              vk::ImageLayout   I_SrcImageLayout,
              vk::ImageLayout   I_DstImageLayout)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_SrcImage != nullptr);
        VISERA_ASSERT(I_DstImage != nullptr);

        constexpr auto Offset = vk::Offset3D{0, 0, 0};
        const auto&  SrcExtent = I_SrcImage->GetExtent();
        vk::Offset3D SrcRange(SrcExtent.width, SrcExtent.height, SrcExtent.depth);
        const auto&  DstExtent = I_DstImage->GetExtent();
        vk::Offset3D DstRange(DstExtent.width, DstExtent.height, DstExtent.depth);

        const auto BlitRegion = vk::ImageBlit2{}
            .setSrcSubresource (I_SrcImage->GetSubresourceLayers(0))
            .setSrcOffsets     ({Offset, SrcRange})
            .setDstSubresource (I_DstImage->GetSubresourceLayers(0))
            .setDstOffsets     ({Offset, DstRange});
        const auto BlitInfo = vk::BlitImageInfo2{}
            .setSrcImage       (I_SrcImage->GetHandle())
            .setSrcImageLayout (I_SrcImageLayout)
            .setDstImage       (I_DstImage->GetHandle())
            .setDstImageLayout (I_DstImageLayout)
            .setRegionCount    (1)
            .setPRegions       (&BlitRegion)
            .setFilter         (I_Filter);
        Handle.blitImage2(BlitInfo);
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>::
    CopyBufferToImage(FVulkanBuffer* I_SrcBuffer,
                      FVulkanImage*  I_DstImage,
                      vk::ImageLayout I_DstImageLayout)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_SrcBuffer != nullptr);
        VISERA_ASSERT(I_DstImage != nullptr);
        VISERA_ASSERT(I_SrcBuffer->GetMemorySize() <= I_DstImage->GetMemorySize());

        const auto ImageSubresourceRange = vk::ImageSubresourceLayers{}
            .setAspectMask     (vk::ImageAspectFlagBits::eColor)
            .setMipLevel      (0)
            .setBaseArrayLayer (0)
            .setLayerCount     (1);
        auto CopyRegion = vk::BufferImageCopy2{}
            .setBufferOffset      (0)
            .setBufferRowLength   (0)
            .setBufferImageHeight (0)
            .setImageSubresource  (ImageSubresourceRange)
            .setImageOffset       ({0, 0, 0})
            .setImageExtent       (I_DstImage->GetExtent());
        auto CopyInfo = vk::CopyBufferToImageInfo2{}
            .setSrcBuffer       (I_SrcBuffer->GetHandle())
            .setDstImage        (I_DstImage->GetHandle())
            .setDstImageLayout  (I_DstImageLayout)
            .setRegionCount     (1)
            .setPRegions        (&CopyRegion);
        Handle.copyBufferToImage2(CopyInfo);
    }

    // ========== Compute implementations ==========
    void FVulkanCommandBuffer<EVulkanQueueFamily::Compute>::
    ConvertImageLayout(FVulkanImage*           I_Image,
                       vk::ImageLayout         I_OldLayout,
                       vk::ImageLayout         I_NewLayout,
                       EVulkanComputeStage     I_SrcStage,
                       EVulkanComputeAccess    I_SrcAccess,
                       EVulkanComputeStage     I_DstStage,
                       EVulkanComputeAccess    I_DstAccess)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_Image != nullptr);

        auto OldLayout = I_OldLayout;
        auto NewLayout = I_NewLayout;
        if (NewLayout == vk::ImageLayout::eUndefined || OldLayout == NewLayout)
        { return; }

        const auto ImageHandle = I_Image->GetHandle();
        auto ImageSubresourceRange = vk::ImageSubresourceRange{}
            .setAspectMask    (vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel  (0)
            .setLevelCount    (I_Image->GetMipmapLevels())
            .setBaseArrayLayer(0)
            .setLayerCount    (I_Image->GetArrayLayers());

        auto Barrier = vk::ImageMemoryBarrier2{}
            .setSrcStageMask       (TypeCast(I_SrcStage))
            .setSrcAccessMask      (TypeCast(I_SrcAccess))
            .setDstStageMask       (TypeCast(I_DstStage))
            .setDstAccessMask      (TypeCast(I_DstAccess))
            .setOldLayout          (OldLayout)
            .setNewLayout          (NewLayout)
            .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setImage              (ImageHandle)
            .setSubresourceRange   (ImageSubresourceRange);
        I_Image->ConvertLayout(Handle, Barrier);
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Compute>::
    PushConstants(const void* I_Data,
                  UInt32      I_Offset,
                  UInt32      I_Size)
    {
        VISERA_ASSERT(IsInsideComputePipeline());
        VISERA_ASSERT((I_Offset % 4) == 0);
        VISERA_ASSERT((I_Size   % 4) == 0);
        VISERA_ASSERT(CurrentComputePipeline != nullptr);

        auto* PipelineLayout = CurrentComputePipeline->GetLayout();
        auto StageFlags = vk::ShaderStageFlagBits::eCompute;

        const auto Info = vk::PushConstantsInfo{}
            .setLayout     (PipelineLayout->GetHandle())
            .setStageFlags (StageFlags)
            .setOffset     (I_Offset)
            .setSize       (I_Size)
            .setPValues    (I_Data);
        Handle.pushConstants2(Info);
    }

    void FVulkanCommandBuffer<EVulkanQueueFamily::Compute>::
    BindDescriptorSet(UInt32                I_SetIndex,
                      FVulkanDescriptorSet* I_DescriptorSet)
    {
        VISERA_ASSERT(IsInsideComputePipeline());
        VISERA_ASSERT(I_DescriptorSet != nullptr);
        VISERA_ASSERT(CurrentComputePipeline != nullptr);

        auto* PipelineLayout = CurrentComputePipeline->GetLayout();
        auto StageFlags = vk::ShaderStageFlagBits::eCompute;

        const auto DescriptorSet = I_DescriptorSet->GetHandle();

        const auto BindInfo = vk::BindDescriptorSetsInfo{}
            .setLayout             (PipelineLayout->GetHandle())
            .setStageFlags         (StageFlags)
            .setFirstSet           (I_SetIndex)
            .setDescriptorSetCount (1)
            .setPDescriptorSets    (&DescriptorSet);
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
                      FVulkanImage*  I_DstImage,
                      vk::ImageLayout I_DstImageLayout)
    {
        VISERA_ASSERT(IsRecording());
        VISERA_ASSERT(I_SrcBuffer != nullptr);
        VISERA_ASSERT(I_DstImage != nullptr);
        VISERA_ASSERT(I_SrcBuffer->GetMemorySize() <= I_DstImage->GetMemorySize());

        const auto ImageSubresourceRange = vk::ImageSubresourceLayers{}
            .setAspectMask     (vk::ImageAspectFlagBits::eColor)
            .setMipLevel      (0)
            .setBaseArrayLayer (0)
            .setLayerCount     (1);
        auto CopyRegion = vk::BufferImageCopy2{}
            .setBufferOffset      (0)
            .setBufferRowLength   (0)
            .setBufferImageHeight (0)
            .setImageSubresource  (ImageSubresourceRange)
            .setImageOffset       ({0, 0, 0})
            .setImageExtent       (I_DstImage->GetExtent());
        auto CopyInfo = vk::CopyBufferToImageInfo2{}
            .setSrcBuffer       (I_SrcBuffer->GetHandle())
            .setDstImage        (I_DstImage->GetHandle())
            .setDstImageLayout   (I_DstImageLayout)
            .setRegionCount     (1)
            .setPRegions        (&CopyRegion);
        Handle.copyBufferToImage2(CopyInfo);
    }
}
