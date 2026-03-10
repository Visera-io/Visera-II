module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.CommandList;
#define VISERA_MODULE_NAME "RHI.CommandList"
import Visera.Runtime.RHI.Common;
import Visera.Runtime.RHI.Registry;
import Visera.Runtime.RHI.Barrier;
import Visera.Runtime.RHI.Resource;
import Visera.Core.Log;
import Visera.Core.OS.Memory;
import Visera.Core.Math.Color;
import Visera.Core.Containers.Array;

#define RHI_COMMAND_LIST \
    RHI_COMMAND(TransitionTexture) \
    RHI_COMMAND(MemoryBarrier) \
    RHI_COMMAND(BufferBarrier) \
    RHI_COMMAND(WriteBuffer) \
    RHI_COMMAND(CopyBufferToImage) \
    RHI_COMMAND(CopyImageToBuffer) \
    RHI_COMMAND(ClearColorImage) \
    RHI_COMMAND(BlitImage) \
    RHI_COMMAND(CopyImage) \
    RHI_COMMAND(EnterRenderPass) \
    RHI_COMMAND(SetViewport) \
    RHI_COMMAND(SetScissor) \
    RHI_COMMAND(LeaveRenderPass) \
    RHI_COMMAND(BindVertexBuffer) \
    RHI_COMMAND(BindDescriptorSet) \
    RHI_COMMAND(PushConstants) \
    RHI_COMMAND(Draw) \
    RHI_COMMAND(DrawIndexed) \
    RHI_COMMAND(EnterComputePass) \
    RHI_COMMAND(LeaveComputePass) \
    RHI_COMMAND(Dispatch)

export namespace Visera
{
    enum class ERHICommandType : UInt16
    {
#define RHI_COMMAND(Name) Name,
        RHI_COMMAND_LIST
#undef RHI_COMMAND
    };

    // Command view returned by iterator
    struct FRHICommandView
    {
        ERHICommandType Type;
        const FByte* PayloadPtrAligned; // Already aligned payload start
        UInt16       PayloadBytes;      // = TotalBytes - PayloadOff
    };

    class VISERA_RUNTIME_API FRHICommandList
    {
        static constexpr UInt64 CommandAlignment = 8;

    private:

        struct alignas(8) FCommandHeader
        {
            ERHICommandType Type;       // Command type
            UInt16       PayloadOff; // Offset from header start to payload start
            UInt16       TotalBytes; // Total bytes from header start to command end
            UInt16       Pad;        // Reserved / padding
        };
        static_assert(sizeof(FCommandHeader)  == 8);
        static_assert(alignof(FCommandHeader) == 8);

    public:
        PROFILING_ONLY_FIELD(
        struct FProfilingMetrics
        {
            UInt64 PeakCommandCount {0};
            UInt64 PeakBufferSizeBytes {0};
            UInt64 PeakBufferCapacityBytes {0};
            UInt64 PeakCommandBytes {0};
            ERHICommandType PeakCommandType {ERHICommandType::TransitionTexture};
        } ProfilingMetrics {};
        )

        struct alignas(8) FWriteBuffer
        {
            FRHIBufferHandle TargetBuffer;
            FRHIBufferHandle StagingBuffer;
        };
        void inline
        WriteBuffer(const FRHIBufferID& I_TargetBuffer, const FRHIBufferID& I_StagingBuffer);

        struct alignas(8) FEnterRenderPass
        {
            FRHIRenderPassHandle      RenderPass;
            UInt8                     ColorTargetCount { 0 };
            struct alignas(8) FColorAttachmentSlot
            {
                FRHITextureHandle      ColorTexture    {};
                ERHIAttachmentLoadOp   ColorLoadOp     { ERHIAttachmentLoadOp::Clear  };
                ERHIAttachmentStoreOp  ColorStoreOp    { ERHIAttachmentStoreOp::Store };
                FLinearColor           ColorClearValue { FLinearColor::Purple() };
            };
            FColorAttachmentSlot ColorSlots[kMaxColorAttachments];
            struct alignas(8) FDepthStencilSlot
            {
                FRHITextureHandle       DepthStencilTexture {};
                ERHIAttachmentLoadOp   DepthLoadOp        { ERHIAttachmentLoadOp::Clear };
                ERHIAttachmentStoreOp  DepthStoreOp       { ERHIAttachmentStoreOp::Store };
                Float                  DepthClearValue    { 1.0f };
                ERHIAttachmentLoadOp   StencilLoadOp      { ERHIAttachmentLoadOp::DontCare };
                ERHIAttachmentStoreOp  StencilStoreOp     { ERHIAttachmentStoreOp::DontCare };
                UInt32                 StencilClearValue  { 0 };
            };
            FDepthStencilSlot DepthStencilSlot;
        };
        void inline
        EnterRenderPass(const FRHIRenderPassID& I_RenderPass, const FRHIRenderPassAttachments& I_Attachments);

        struct alignas(8) FLeaveRenderPass
        {
            UInt8 _ {0};  // Minimal payload for RecordCommand
        };
        void inline
        LeaveRenderPass();

        struct alignas(8) FBindVertexBuffer
        {
            FRHIBufferHandle Buffer;
            UInt8            Binding   {0};
            UInt64           Offset    {0};
        };
        void inline
        BindVertexBuffer(const FRHIBufferID& I_Buffer, UInt8 I_Binding = 0, UInt64 I_Offset = 0);

        struct alignas(8) FBindDescriptorSet
        {
            FRHIDescriptorSetHandle DescriptorSet;
            UInt32                  SetIndex {0};
        };
        void inline
        BindDescriptorSet(const FRHIDescriptorSetID& I_DescriptorSet, UInt32 I_SetIndex = 0);

        struct alignas(8) FPushConstants
        {
            UInt32 Offset {0};
            UInt32 Size   {0};
            FByte  Data[kMaxPushConstantSize];
        };
        void inline
        PushConstants(const void* I_Data, UInt32 I_Offset, UInt32 I_Size);

        struct alignas(8) FDraw
        {
            UInt32 VertexCount   {0};
            UInt32 InstanceCount {1};
            UInt32 FirstVertex   {0};
            UInt32 FirstInstance {0};
        };
        void inline
        Draw(UInt32 I_VertexCount, UInt32 I_InstanceCount = 1, UInt32 I_FirstVertex = 0, UInt32 I_FirstInstance = 0);

        struct alignas(8) FDrawIndexed
        {
            UInt32 IndexCount    {0};
            UInt32 InstanceCount {1};
            UInt32 FirstIndex    {0};
            Int32  VertexOffset  {0};
            UInt32 FirstInstance {0};
        };
        void inline
        DrawIndexed(UInt32 I_IndexCount, UInt32 I_InstanceCount = 1, UInt32 I_FirstIndex = 0, Int32 I_VertexOffset = 0, UInt32 I_FirstInstance = 0);

        struct alignas(8) FCopyBufferToImage
        {
            FRHIBufferHandle  Buffer;
            FRHITextureHandle Image;
            ERHIImageLayout   InitialLayout;
            ERHIImageLayout   FinalLayout;
        };
        void inline
        CopyBufferToImage(const FRHIBufferID& I_Buffer, const FRHITextureID& I_Texture,
                         ERHIImageLayout I_InitialLayout = ERHIImageLayout::Undefined,
                         ERHIImageLayout I_FinalLayout = ERHIImageLayout::ShaderReadOnly);

        struct alignas(8) FCopyImageToBuffer
        {
            FRHITextureHandle SourceTexture;
            FRHIBufferHandle  DestBuffer;
            UInt64            DestBufferOffset {0};
            FRHIOffset2D      ImageOffset      {0, 0};
            FRHIExtent2D      ImageExtent      {0, 0};
        };
        void inline
        CopyImageToBuffer(const FRHITextureID& I_SourceTexture, const FRHIBufferID& I_DestBuffer,
                         UInt64 I_DestBufferOffset, const FRHIOffset2D& I_ImageOffset,
                         const FRHIExtent2D& I_ImageExtent);

        struct alignas(8) FSetViewport
        {
            FRHIViewport Viewport;
        };
        void inline
        SetViewport(const FRHIViewport& I_Viewport);

        struct alignas(8) FSetScissor
        {
            FRHIScissor Scissor;
        };
        void inline
        SetScissor(const FRHIScissor& I_Scissor);

        struct alignas(8) FTransitionTexturePayload
        {
            FRHITextureHandle Image;
            ERHIImageLayout   OldLayout;
            ERHIImageLayout   NewLayout;
            ERHIPipelineStage SourceStage;
            ERHIPipelineStage DestStage;
            ERHIAccessFlag    SourceAccess;
            ERHIAccessFlag    DestAccess;
        };
        void inline
        TransitionTexture(const FRHIImageBarrier& I_Barrier);

        struct alignas(8) FMemoryBarrierPayload
        {
            ERHIPipelineStage SourceStage;
            ERHIPipelineStage DestStage;
            ERHIAccessFlag    SourceAccess;
            ERHIAccessFlag    DestAccess;
        };
        void inline
        MemoryBarrier(const FRHIMemoryBarrier& I_Barrier);

        struct alignas(8) FBufferBarrierPayload
        {
            FRHIBufferHandle  Buffer;
            UInt64            Offset;
            UInt64            Size;
            ERHIPipelineStage SourceStage;
            ERHIPipelineStage DestStage;
            ERHIAccessFlag    SourceAccess;
            ERHIAccessFlag    DestAccess;
        };
        void inline
        BufferBarrier(const FRHIBufferBarrier& I_Barrier);

        struct alignas(8) FClearColorImage
        {
            FRHITextureHandle  Image;
            FRHIClearColor     ClearColor;
            ERHIImageLayout    ImageLayout;
        };
        void inline
        ClearColorImage(const FRHITextureID& I_Texture, FRHIClearColor I_ClearColor,
                       ERHIImageLayout I_ImageLayout = ERHIImageLayout::TransferDst);

        struct alignas(8) FBlitImage
        {
            FRHITextureHandle SrcImage;
            FRHITextureHandle DstImage;
            ERHIFilter        Filter;
            ERHIImageLayout   SrcImageLayout;
            ERHIImageLayout   DstImageLayout;
        };
        void inline
        BlitImage(const FRHITextureID& I_SrcTexture, const FRHITextureID& I_DstTexture, ERHIFilter I_Filter,
                  ERHIImageLayout I_SrcLayout = ERHIImageLayout::TransferSrc,
                  ERHIImageLayout I_DstLayout = ERHIImageLayout::TransferDst);

        struct alignas(8) FCopyImage
        {
            FRHITextureHandle SrcImage;
            FRHITextureHandle DstImage;
            ERHIImageLayout   SrcImageLayout;
            ERHIImageLayout   DstImageLayout;
        };
        void inline
        CopyImage(const FRHITextureID& I_SrcTexture, const FRHITextureID& I_DstTexture,
                  ERHIImageLayout I_SrcLayout = ERHIImageLayout::TransferSrc,
                  ERHIImageLayout I_DstLayout = ERHIImageLayout::TransferDst);

        struct alignas(8) FEnterComputePass
        {
            FRHIComputePassHandle ComputePass;
        };
        void inline
        EnterComputePass(const FRHIComputePassID& I_ComputePass);

        struct alignas(8) FLeaveComputePass
        {
            UInt8 _ {0};
        };
        void inline
        LeaveComputePass();

        struct alignas(8) FDispatch
        {
            UInt32 GroupCountX {0};
            UInt32 GroupCountY {0};
            UInt32 GroupCountZ {0};
        };
        void inline
        Dispatch(UInt32 I_GroupCountX, UInt32 I_GroupCountY, UInt32 I_GroupCountZ);

        PROFILING_ONLY_FIELD(
        [[nodiscard]] const FProfilingMetrics&
        GetProfilingMetrics() const { return ProfilingMetrics; }
        )

        // Check if the command list is empty
        [[nodiscard]] Bool
        IsEmpty() const { return Buffer.IsEmpty(); }
        // Get the raw buffer data (for execution)
        [[nodiscard]] const FByte*
        GetData() const { return Buffer.Data(); }
        // Get the buffer size in bytes
        [[nodiscard]] UInt64
        GetSize() const { return Buffer.GetSize(); }
        void
        Reset() { Buffer.Clear(); CommandCount = 0; }

        void
        ShrinkTo(UInt64 I_MaxCapacity)
        {
            if (Buffer.GetCapacity() > I_MaxCapacity)
            {
                Buffer = TArray<FByte>();
                Buffer.Reserve(I_MaxCapacity);
            }
        }

        // Iterator for range-based for loop
        class VISERA_RUNTIME_API FIterator
        {
        public:
            FIterator(const FByte* I_Data, UInt64 I_Size, UInt64 I_Offset)
                : Data(I_Data)
                , Size(I_Size)
                , Offset(I_Offset)
            {
            }

            [[nodiscard]] FRHICommandView operator*() const
            {
                const UInt64 HeaderOffset = Memory::Align(Offset, FRHICommandList::CommandAlignment);
                if (HeaderOffset + sizeof(FCommandHeader) > Size)
                    return {ERHICommandType{}, nullptr, 0};

                const auto* Header = reinterpret_cast<const FCommandHeader*>(Data + HeaderOffset);

                if (Header->TotalBytes == 0) return {Header->Type, nullptr, 0};
                if (HeaderOffset + Header->TotalBytes > Size) return {Header->Type, nullptr, 0};
                if (Header->PayloadOff < sizeof(FCommandHeader)) return {Header->Type, nullptr, 0};
                if (Header->PayloadOff > Header->TotalBytes) return {Header->Type, nullptr, 0};

                const FByte* PayloadPtr = Data + HeaderOffset + Header->PayloadOff;
                const UInt16 PayloadBytes = static_cast<UInt16>(Header->TotalBytes - Header->PayloadOff);

                return {Header->Type, PayloadPtr, PayloadBytes};
            }

            FIterator& operator++()
            {
                const UInt64 HeaderOffset = Memory::Align(Offset, FRHICommandList::CommandAlignment);
                if (HeaderOffset + sizeof(FCommandHeader) > Size)
                { Offset = Size; return *this; }

                const auto* Header = reinterpret_cast<const FCommandHeader*>(Data + HeaderOffset);

                if (Header->TotalBytes == 0)
                { Offset = Size; return *this; }

                Offset = HeaderOffset + Header->TotalBytes;
                if (Offset >= Size) Offset = Size; // clamp

                return *this;
            }

            [[nodiscard]] Bool operator==(const FIterator& I_Other) const
            {
                return Data == I_Other.Data && Offset == I_Other.Offset;
            }

            [[nodiscard]] Bool operator!=(const FIterator& I_Other) const
            {
                return !(*this == I_Other);
            }

        private:
            const FByte* Data;
            UInt64       Size;
            UInt64       Offset;
        };

        [[nodiscard]] FIterator begin() const
        {
            return FIterator{Buffer.Data(), Buffer.GetSize(), 0};
        }

        [[nodiscard]] FIterator end() const
        {
            return FIterator{Buffer.Data(), Buffer.GetSize(), Buffer.GetSize()};
        }

    private:
        TArray<FByte>  Buffer;
        UInt64         CommandCount = 0;

    public:
        FRHICommandList() = default;
        ~FRHICommandList() = default;
        FRHICommandList(const FRHICommandList&) = default;
        FRHICommandList& operator=(const FRHICommandList&) = default;
        FRHICommandList(FRHICommandList&&) noexcept = default;
        FRHICommandList& operator=(FRHICommandList&&) noexcept = default;

    private:
        // Record a command with payload struct
        template<typename Payload> void
        RecordCommand(ERHICommandType I_Type, const Payload& I_Payload) requires std::is_trivially_copyable_v<Payload>
        {
            // Align header start to CommandAlignment
            const UInt64 HeaderOffset = Memory::Align(Buffer.GetSize(), CommandAlignment);

            // Align payload start to CommandAlignment (fixed alignment, no alignof)
            const UInt64 PayloadStart = Memory::Align(HeaderOffset + sizeof(FCommandHeader), CommandAlignment);
            const UInt64 PayloadEnd = PayloadStart + sizeof(Payload);

            // Align command end to CommandAlignment (optional but recommended)
            const UInt64 CommandEnd = Memory::Align(PayloadEnd, CommandAlignment);

            // Compute offsets
            const UInt64 PayloadOff = PayloadStart - HeaderOffset;
            const UInt64 TotalBytes = CommandEnd - HeaderOffset;

            // Validation
            VISERA_ASSERT(PayloadOff <= TotalBytes);
            VISERA_ASSERT(TotalBytes <= 0xFFFF);
            VISERA_ASSERT(PayloadOff % CommandAlignment == 0);
            VISERA_ASSERT(TotalBytes % CommandAlignment == 0);

            // Resize buffer
            Buffer.Resize(CommandEnd);

            // Write header
            FCommandHeader Header{};
            Header.Type = I_Type;
            Header.PayloadOff = static_cast<UInt16>(PayloadOff);
            Header.TotalBytes = static_cast<UInt16>(TotalBytes);
            Header.Pad = 0;

            Memory::Memcpy(Buffer.Data() + HeaderOffset, &Header, sizeof(FCommandHeader));
            Memory::Memcpy(Buffer.Data() + PayloadStart, &I_Payload, sizeof(Payload));

            ++CommandCount;
            PROFILING_ONLY_FIELD(
            if (CommandCount > ProfilingMetrics.PeakCommandCount)
                ProfilingMetrics.PeakCommandCount = CommandCount;
            if (Buffer.GetSize() > ProfilingMetrics.PeakBufferSizeBytes)
                ProfilingMetrics.PeakBufferSizeBytes = Buffer.GetSize();
            if (Buffer.GetCapacity() > ProfilingMetrics.PeakBufferCapacityBytes)
                ProfilingMetrics.PeakBufferCapacityBytes = Buffer.GetCapacity();
            if (TotalBytes > ProfilingMetrics.PeakCommandBytes)
            {
                ProfilingMetrics.PeakCommandBytes = TotalBytes;
                ProfilingMetrics.PeakCommandType  = I_Type;
            }
            );
        }
    };

    void FRHICommandList::
    WriteBuffer(const FRHIBufferID& I_TargetBuffer, const FRHIBufferID& I_StagingBuffer)
    {
        if(I_TargetBuffer.IsNull())
        {
            LOG_ERROR("WriteBuffer: TargetBuffer is null");
            return;
        }
        if(I_StagingBuffer.IsNull())
        {
            LOG_ERROR("WriteBuffer: StagingBuffer is null");
            return;
        }
        const auto TargetBufferHandle  = I_TargetBuffer.GetHandle();
        const auto StagingBufferHandle = I_StagingBuffer.GetHandle();
        RecordCommand(ERHICommandType::WriteBuffer, FWriteBuffer
        {
            .TargetBuffer  = TargetBufferHandle,
            .StagingBuffer = StagingBufferHandle
        });
    }

    void FRHICommandList::
    TransitionTexture(const FRHIImageBarrier& I_Barrier)
    {
        if(I_Barrier.Image.IsNull())
        {
            LOG_ERROR("TransitionTexture: Image is null");
            return;
        }
        const auto Handle = I_Barrier.Image.GetHandle();
        RecordCommand(ERHICommandType::TransitionTexture, FTransitionTexturePayload
        {
            .Image        = Handle,
            .OldLayout    = I_Barrier.OldLayout,
            .NewLayout    = I_Barrier.NewLayout,
            .SourceStage  = I_Barrier.MemoryBarrier.SourceStage,
            .DestStage    = I_Barrier.MemoryBarrier.DestStage,
            .SourceAccess = I_Barrier.MemoryBarrier.SourceAccess,
            .DestAccess   = I_Barrier.MemoryBarrier.DestAccess,
        });
    }

    void FRHICommandList::
    MemoryBarrier(const FRHIMemoryBarrier& I_Barrier)
    {
        RecordCommand(ERHICommandType::MemoryBarrier, FMemoryBarrierPayload
        {
            .SourceStage  = I_Barrier.SourceStage,
            .DestStage    = I_Barrier.DestStage,
            .SourceAccess = I_Barrier.SourceAccess,
            .DestAccess   = I_Barrier.DestAccess,
        });
    }

    void FRHICommandList::
    BufferBarrier(const FRHIBufferBarrier& I_Barrier)
    {
        if(I_Barrier.Buffer.IsNull())
        {
            LOG_ERROR("BufferBarrier: Buffer is null");
            return;
        }
        const auto Handle = I_Barrier.Buffer.GetHandle();
        RecordCommand(ERHICommandType::BufferBarrier, FBufferBarrierPayload
        {
            .Buffer       = Handle,
            .Offset       = I_Barrier.Offset,
            .Size         = I_Barrier.Size,
            .SourceStage  = I_Barrier.MemoryBarrier.SourceStage,
            .DestStage    = I_Barrier.MemoryBarrier.DestStage,
            .SourceAccess = I_Barrier.MemoryBarrier.SourceAccess,
            .DestAccess   = I_Barrier.MemoryBarrier.DestAccess,
        });
    }

    void FRHICommandList::
    CopyBufferToImage(const FRHIBufferID& I_Buffer, const FRHITextureID& I_Texture,
                     ERHIImageLayout I_InitialLayout, ERHIImageLayout I_FinalLayout)
    {
        if(I_Buffer.IsNull())
        {
            LOG_ERROR("CopyBufferToImage: Buffer is null");
            return;
        }
        if(I_Texture.IsNull())
        {
            LOG_ERROR("CopyBufferToImage: Texture is null");
            return;
        }
        const auto BufferHandle = I_Buffer.GetHandle();
        const auto ImageHandle  = I_Texture.GetHandle();
        RecordCommand(ERHICommandType::CopyBufferToImage, FCopyBufferToImage
        {
            .Buffer         = BufferHandle,
            .Image          = ImageHandle,
            .InitialLayout  = I_InitialLayout,
            .FinalLayout    = I_FinalLayout,
        });
    }

    void FRHICommandList::
    CopyImageToBuffer(const FRHITextureID& I_SourceTexture, const FRHIBufferID& I_DestBuffer,
                     UInt64 I_DestBufferOffset, const FRHIOffset2D& I_ImageOffset,
                     const FRHIExtent2D& I_ImageExtent)
    {
        if(I_SourceTexture.IsNull())
        {
            LOG_ERROR("CopyImageToBuffer: SourceTexture is null");
            return;
        }
        if(I_DestBuffer.IsNull())
        {
            LOG_ERROR("CopyImageToBuffer: DestBuffer is null");
            return;
        }
        const auto SourceHandle = I_SourceTexture.GetHandle();
        const auto DestHandle   = I_DestBuffer.GetHandle();
        RecordCommand(ERHICommandType::CopyImageToBuffer, FCopyImageToBuffer
        {
            .SourceTexture     = SourceHandle,
            .DestBuffer        = DestHandle,
            .DestBufferOffset  = I_DestBufferOffset,
            .ImageOffset       = I_ImageOffset,
            .ImageExtent       = I_ImageExtent,
        });
    }

    void FRHICommandList::
    SetViewport(const FRHIViewport& I_Viewport)
    {
        RecordCommand(ERHICommandType::SetViewport, FSetViewport
        {
            .Viewport = I_Viewport,
        });
    }

    void FRHICommandList::
    SetScissor(const FRHIScissor& I_Scissor)
    {
        RecordCommand(ERHICommandType::SetScissor, FSetScissor
        {
            .Scissor = I_Scissor,
        });
    }

    void FRHICommandList::
    ClearColorImage(const FRHITextureID& I_Texture, FRHIClearColor I_ClearColor,
                   ERHIImageLayout I_ImageLayout)
    {
        if(I_Texture.IsNull())
        {
            LOG_ERROR("ClearColorImage: Texture is null");
            return;
        }
        const auto Handle = I_Texture.GetHandle();
        RecordCommand(ERHICommandType::ClearColorImage, FClearColorImage
        {
            .Image       = Handle,
            .ClearColor  = I_ClearColor,
            .ImageLayout = I_ImageLayout,
        });
    }

    void FRHICommandList::
    BlitImage(const FRHITextureID& I_SrcTexture, const FRHITextureID& I_DstTexture, ERHIFilter I_Filter,
              ERHIImageLayout I_SrcLayout, ERHIImageLayout I_DstLayout)
    {
        if(I_SrcTexture.IsNull())
        {
            LOG_ERROR("BlitImage: SrcTexture is null");
            return;
        }
        if(I_DstTexture.IsNull())
        {
            LOG_ERROR("BlitImage: DstTexture is null");
            return;
        }
        const auto SrcHandle = I_SrcTexture.GetHandle();
        const auto DstHandle = I_DstTexture.GetHandle();
        RecordCommand(ERHICommandType::BlitImage, FBlitImage
        {
            .SrcImage       = SrcHandle,
            .DstImage       = DstHandle,
            .Filter         = I_Filter,
            .SrcImageLayout = I_SrcLayout,
            .DstImageLayout = I_DstLayout,
        });
    }

    void FRHICommandList::
    CopyImage(const FRHITextureID& I_SrcTexture, const FRHITextureID& I_DstTexture,
              ERHIImageLayout I_SrcLayout, ERHIImageLayout I_DstLayout)
    {
        if(I_SrcTexture.IsNull())
        {
            LOG_ERROR("CopyImage: SrcTexture is null");
            return;
        }
        if(I_DstTexture.IsNull())
        {
            LOG_ERROR("CopyImage: DstTexture is null");
            return;
        }
        const auto SrcHandle = I_SrcTexture.GetHandle();
        const auto DstHandle = I_DstTexture.GetHandle();
        RecordCommand(ERHICommandType::CopyImage, FCopyImage
        {
            .SrcImage       = SrcHandle,
            .DstImage       = DstHandle,
            .SrcImageLayout = I_SrcLayout,
            .DstImageLayout = I_DstLayout,
        });
    }

    void FRHICommandList::
    EnterRenderPass(const FRHIRenderPassID& I_RenderPass, const FRHIRenderPassAttachments& I_Attachments)
    {
        if(I_RenderPass.IsNull())
        {
            LOG_ERROR("EnterRenderPass: RenderPass is null");
            return;
        }
        const auto Handle = I_RenderPass.GetHandle();
        FEnterRenderPass Payload{};
        Payload.RenderPass        = Handle;
        Payload.ColorTargetCount  = static_cast<UInt8>(I_Attachments.ColorAttachments.GetSize());
        if (Payload.ColorTargetCount > kMaxColorAttachments)
        {
            LOG_ERROR("EnterRenderPass: ColorTargetCount {} exceeds kMaxColorAttachments ({}), clamping.",
                      Payload.ColorTargetCount, kMaxColorAttachments);
            Payload.ColorTargetCount = static_cast<UInt8>(kMaxColorAttachments);
        }
        for (UInt32 i = 0; i < Payload.ColorTargetCount; ++i)
        {
            const auto& Src = I_Attachments.ColorAttachments[i];
            auto& Dst = Payload.ColorSlots[i];
            Dst.ColorTexture    = Src.Texture;
            Dst.ColorLoadOp     = Src.LoadOp;
            Dst.ColorStoreOp    = Src.StoreOp;
            Dst.ColorClearValue = Src.ColorClearValue;
        }
        const auto& Ds = I_Attachments.DepthStencilAttachment;
        Payload.DepthStencilSlot.DepthStencilTexture = Ds.Texture;
        Payload.DepthStencilSlot.DepthLoadOp          = Ds.DepthLoadOp;
        Payload.DepthStencilSlot.DepthStoreOp         = Ds.DepthStoreOp;
        Payload.DepthStencilSlot.DepthClearValue      = Ds.DepthClearValue;
        Payload.DepthStencilSlot.StencilLoadOp        = Ds.StencilLoadOp;
        Payload.DepthStencilSlot.StencilStoreOp       = Ds.StencilStoreOp;
        Payload.DepthStencilSlot.StencilClearValue    = Ds.StencilClearValue;
        RecordCommand(ERHICommandType::EnterRenderPass, Payload);
    }

    void FRHICommandList::
    LeaveRenderPass()
    {
        RecordCommand(ERHICommandType::LeaveRenderPass, FLeaveRenderPass{});
    }

    void FRHICommandList::
    BindVertexBuffer(const FRHIBufferID& I_Buffer, UInt8 I_Binding, UInt64 I_Offset)
    {
        if(I_Buffer.IsNull())
        {
            LOG_ERROR("BindVertexBuffer: Buffer is null");
            return;
        }
        const auto Handle = I_Buffer.GetHandle();
        RecordCommand(ERHICommandType::BindVertexBuffer, FBindVertexBuffer
        {
            .Buffer   = Handle,
            .Binding  = I_Binding,
            .Offset   = I_Offset,
        });
    }

    void FRHICommandList::
    BindDescriptorSet(const FRHIDescriptorSetID& I_DescriptorSet, UInt32 I_SetIndex)
    {
        if(I_DescriptorSet.IsNull())
        {
            LOG_ERROR("BindDescriptorSet: DescriptorSet is null");
            return;
        }
        const auto Handle = I_DescriptorSet.GetHandle();
        RecordCommand(ERHICommandType::BindDescriptorSet, FBindDescriptorSet
        {
            .DescriptorSet = Handle,
            .SetIndex      = I_SetIndex,
        });
    }

    void FRHICommandList::
    PushConstants(const void* I_Data, UInt32 I_Offset, UInt32 I_Size)
    {
        if (I_Data == nullptr || I_Size == 0 || I_Size > kMaxPushConstantSize)
        {
            if (I_Size > kMaxPushConstantSize)
            { LOG_ERROR("PushConstants: Size {} exceeds kMaxPushConstantSize {}.", I_Size, kMaxPushConstantSize); }
            return;
        }
        FPushConstants Payload
        {
            .Offset = I_Offset,
            .Size   = I_Size,
        };
        Memory::Memcpy(Payload.Data, I_Data, I_Size);
        RecordCommand(ERHICommandType::PushConstants, Payload);
    }

    void FRHICommandList::
    Draw(UInt32 I_VertexCount, UInt32 I_InstanceCount, UInt32 I_FirstVertex, UInt32 I_FirstInstance)
    {
        RecordCommand(ERHICommandType::Draw, FDraw
        {
            .VertexCount   = I_VertexCount,
            .InstanceCount = I_InstanceCount,
            .FirstVertex   = I_FirstVertex,
            .FirstInstance = I_FirstInstance,
        });
    }

    void FRHICommandList::
    DrawIndexed(UInt32 I_IndexCount, UInt32 I_InstanceCount, UInt32 I_FirstIndex, Int32 I_VertexOffset, UInt32 I_FirstInstance)
    {
        RecordCommand(ERHICommandType::DrawIndexed, FDrawIndexed
        {
            .IndexCount    = I_IndexCount,
            .InstanceCount = I_InstanceCount,
            .FirstIndex    = I_FirstIndex,
            .VertexOffset  = I_VertexOffset,
            .FirstInstance = I_FirstInstance,
        });
    }

    void FRHICommandList::
    EnterComputePass(const FRHIComputePassID& I_ComputePass)
    {
        if(I_ComputePass.IsNull())
        {
            LOG_ERROR("EnterComputePass: ComputePass is null");
            return;
        }
        const auto Handle = I_ComputePass.GetHandle();
        RecordCommand(ERHICommandType::EnterComputePass, FEnterComputePass
        {
            .ComputePass = Handle,
        });
    }

    void FRHICommandList::
    LeaveComputePass()
    {
        RecordCommand(ERHICommandType::LeaveComputePass, FLeaveComputePass{});
    }

    void FRHICommandList::
    Dispatch(UInt32 I_GroupCountX, UInt32 I_GroupCountY, UInt32 I_GroupCountZ)
    {
        RecordCommand(ERHICommandType::Dispatch, FDispatch
        {
            .GroupCountX = I_GroupCountX,
            .GroupCountY = I_GroupCountY,
            .GroupCountZ = I_GroupCountZ,
        });
    }
}