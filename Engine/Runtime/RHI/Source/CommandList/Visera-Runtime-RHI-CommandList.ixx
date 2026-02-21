module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.CommandList;
#define VISERA_MODULE_NAME "RHI.CommandList"
import Visera.Runtime.RHI.Common;
import Visera.Runtime.RHI.Registry;
import Visera.Core.Log;
import Visera.Core.OS.Memory;
import Visera.Core.Containers.Array;

export namespace Visera
{
    enum class ECommandType : UInt16
    {
        ConvertImageLayout,
        WriteBuffer,
        CopyBufferToImage,
        ClearColorImage,
        BlitImage,
        BlitToSwapChain,
        EnterRenderPass,
        SetViewport,
        SetScissor,
        LeaveRenderPass,
        BindVertexBuffer,
        BindDescriptorSet,
        Draw,
        DrawIndexed,
    };

    // Command view returned by iterator
    struct FRHICommandView
    {
        ECommandType Type;
        const FByte* PayloadPtrAligned; // Already aligned payload start
        UInt16       PayloadBytes;      // = TotalBytes - PayloadOff
    };

    class VISERA_RUNTIME_API FRHICommandList
    {
        static constexpr UInt64 CommandAlignment = 8;
        static inline constexpr UInt64 InlineArenaBytes = 32_KB;
        PROFILING_ONLY_FIELD(
        struct FProfilingMetrics
        {
            UInt64 PeakCommandCount {0};
            UInt64 PeakBufferSizeBytes {0};
            UInt64 PeakBufferCapacityBytes {0};
            UInt64 PeakCommandBytes {0};
            ECommandType PeakCommandType {ECommandType::ConvertImageLayout};
            UInt64 InlineOverflowEvents {0};
            UInt64 PeakOverInlineBytes {0};
            UInt64 RecommendedInlineBytes {InlineArenaBytes};
        } ProfilingMetrics {};
        );

    public:
        /// Target swap chain for execution. Set by FRHI::Execute(); do not set directly.
        FRHISwapChainID TargetSwapChain { 0 };  // Index into FRHI's swap chain array; 0 = primary

    private:

        struct alignas(8) FCommandHeader
        {
            ECommandType Type;       // Command type
            UInt16       PayloadOff; // Offset from header start to payload start
            UInt16       TotalBytes; // Total bytes from header start to command end
            UInt16       Pad;        // Reserved / padding
        };
        static_assert(sizeof(FCommandHeader)  == 8);
        static_assert(alignof(FCommandHeader) == 8);

    public:
        struct FWriteBuffer
        {
            FRHIBufferHandle TargetBuffer;
            FRHIBufferHandle StagingBuffer;
        };
        void inline
        WriteBuffer(const FRHIBufferID& I_TargetBuffer, const FRHIBufferID& I_StagingBuffer);

        struct FEnterRenderPass
        {
            FRHIRenderPassHandle RenderPass;
        };
        void inline
        EnterRenderPass(const FRHIRenderPassID& I_RenderPass);

        struct FLeaveRenderPass
        {
            UInt8 _ {0};  // Minimal payload for RecordCommand
        };
        void inline
        LeaveRenderPass();

        struct FBindVertexBuffer
        {
            FRHIBufferHandle Buffer;
            UInt8            Binding   {0};
            UInt64           Offset    {0};
        };
        void inline
        BindVertexBuffer(const FRHIBufferID& I_Buffer, UInt8 I_Binding = 0, UInt64 I_Offset = 0);

        struct FBindDescriptorSet
        {
            FRHIDescriptorSetHandle DescriptorSet;
            UInt32                  SetIndex {0};
        };
        void inline
        BindDescriptorSet(const FRHIDescriptorSetID& I_DescriptorSet, UInt32 I_SetIndex = 0);

        struct FDraw
        {
            UInt32 VertexCount   {0};
            UInt32 InstanceCount {1};
            UInt32 FirstVertex   {0};
            UInt32 FirstInstance {0};
        };
        void inline
        Draw(UInt32 I_VertexCount, UInt32 I_InstanceCount = 1, UInt32 I_FirstVertex = 0, UInt32 I_FirstInstance = 0);

        struct FDrawIndexed
        {
            UInt32 IndexCount    {0};
            UInt32 InstanceCount {1};
            UInt32 FirstIndex    {0};
            Int32  VertexOffset  {0};
            UInt32 FirstInstance {0};
        };
        void inline
        DrawIndexed(UInt32 I_IndexCount, UInt32 I_InstanceCount = 1, UInt32 I_FirstIndex = 0, Int32 I_VertexOffset = 0, UInt32 I_FirstInstance = 0);

        struct FCopyBufferToImage
        {
            FRHIBufferHandle  Buffer;
            FRHITextureHandle Image;
        };
        void inline
        CopyBufferToImage(const FRHIBufferID& I_Buffer, const FRHITextureID& I_Texture);

        struct FSetViewport
        {
            FRHIViewport Viewport;
        };
        void inline
        SetViewport(const FRHIViewport& I_Viewport);

        struct FSetScissor
        {
            FRHIScissor Scissor;
        };
        void inline
        SetScissor(const FRHIScissor& I_Scissor);

        struct FConvertImageLayout
        {
            FRHITextureHandle   Image;
            ERHIImageLayout NewLayout;
        };
        void inline
        ConvertImageLayout(const FRHITextureID& I_Texture, ERHIImageLayout I_NewLayout);

        struct FClearColorImage
        {
            FRHITextureHandle  Image;
            FRHIClearColor ClearColor;
        };
        void inline
        ClearColorImage(const FRHITextureID& I_Texture, FRHIClearColor I_ClearColor);

        struct FBlitImage
        {
            FRHITextureHandle SrcImage;
            FRHITextureHandle DstImage;
            ERHIFilter    Filter;
        };
        void inline
        BlitImage(const FRHITextureID& I_SrcTexture, const FRHITextureID& I_DstTexture, ERHIFilter I_Filter);

        struct FBlitToSwapChain
        {
            FRHITextureHandle Image;
            ERHIFilter    Filter;
        };
        void inline
        BlitToSwapChain(const FRHITextureID& I_Texture, ERHIFilter I_Filter);

        // Check if the command list is empty
        [[nodiscard]] Bool
        IsEmpty() const { return Buffer.IsEmpty(); }
        // Get the raw buffer data (for execution)
        [[nodiscard]] const FByte*
        GetData() const { return Buffer.Data(); }
        // Get the buffer size in bytes
        [[nodiscard]] UInt64
        GetSize() const { return Buffer.GetSize(); }
        /// Clear recorded commands. Does not reset MemoryCache so that Buffer's
        /// storage (which may come from the arena) is not freed while still in use.
        void
        Reset() { Buffer.Clear(); CommandCount = 0; }

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
                    return {ECommandType{}, nullptr, 0};

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
        Memory::TMonotonicArena<InlineArenaBytes> MemoryCache;
        TPMRArray<FByte>               Buffer;
        UInt64                         CommandCount = 0;

    public:
        FRHICommandList() : MemoryCache(), Buffer(&MemoryCache.Get()) { }

        FRHICommandList(const FRHICommandList& I_Other)
            : MemoryCache(), Buffer(&MemoryCache.Get()), CommandCount(I_Other.CommandCount)
        {
            Buffer.Resize(I_Other.Buffer.GetSize());
            if (I_Other.Buffer.GetSize() > 0)
            { Memory::Memcpy(Buffer.Data(), I_Other.Buffer.Data(), I_Other.Buffer.GetSize()); }
        }

    private:
        // Record a command with payload struct
        template<typename Payload> void
        RecordCommand(ECommandType I_Type, const Payload& I_Payload) requires std::is_trivially_copyable_v<Payload>
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
            {
                ProfilingMetrics.PeakCommandCount = CommandCount;
                LOG_INFO("[Profiling] CommandList peak command_count={} (last_type={}).",
                    ProfilingMetrics.PeakCommandCount,
                    I_Type);
            }
            if (Buffer.GetSize() > ProfilingMetrics.PeakBufferSizeBytes)
            {
                ProfilingMetrics.PeakBufferSizeBytes = Buffer.GetSize();
                LOG_INFO("[Profiling] CommandList peak buffer_size={} bytes (commands={}).",
                    ProfilingMetrics.PeakBufferSizeBytes,
                    CommandCount);
            }
            if (Buffer.GetCapacity() > ProfilingMetrics.PeakBufferCapacityBytes)
            {
                ProfilingMetrics.PeakBufferCapacityBytes = Buffer.GetCapacity();
                LOG_INFO("[Profiling] CommandList peak buffer_capacity={} bytes.",
                    ProfilingMetrics.PeakBufferCapacityBytes);
                if (ProfilingMetrics.PeakBufferCapacityBytes > InlineArenaBytes)
                {
                    const UInt64 OverInlineBytes = ProfilingMetrics.PeakBufferCapacityBytes - InlineArenaBytes;
                    ++ProfilingMetrics.InlineOverflowEvents;
                    if (OverInlineBytes > ProfilingMetrics.PeakOverInlineBytes)
                    { ProfilingMetrics.PeakOverInlineBytes = OverInlineBytes; }
                    ProfilingMetrics.RecommendedInlineBytes = ProfilingMetrics.PeakBufferCapacityBytes;
                    LOG_WARN("[Profiling] CommandList inline arena pressure: inline={} bytes, peak_capacity={} bytes, over_by={} bytes, recommended_inline={} bytes.",
                        InlineArenaBytes,
                        ProfilingMetrics.PeakBufferCapacityBytes,
                        OverInlineBytes,
                        ProfilingMetrics.RecommendedInlineBytes);
                }
            }
            if (TotalBytes > ProfilingMetrics.PeakCommandBytes)
            {
                ProfilingMetrics.PeakCommandBytes = TotalBytes;
                ProfilingMetrics.PeakCommandType  = I_Type;
                LOG_INFO("[Profiling] CommandList peak command_bytes={} (type={}).",
                    ProfilingMetrics.PeakCommandBytes,
                    ProfilingMetrics.PeakCommandType);
            }
            );
        }
    };

    void FRHICommandList::
    WriteBuffer(const FRHIBufferID& I_TargetBuffer, const FRHIBufferID& I_StagingBuffer)
    {
        const auto TargetBufferHandle  = I_TargetBuffer.GetHandle();
        const auto StagingBufferHandle = I_StagingBuffer.GetHandle();
        VISERA_ASSERT(TargetBufferHandle  != FRHIBufferHandle{});
        VISERA_ASSERT(StagingBufferHandle != FRHIBufferHandle{});
        RecordCommand(ECommandType::WriteBuffer, FWriteBuffer
        {
            .TargetBuffer  = TargetBufferHandle,
            .StagingBuffer = StagingBufferHandle
        });
    }

    void FRHICommandList::
    ConvertImageLayout(const FRHITextureID& I_Texture, ERHIImageLayout I_NewLayout)
    {
        const auto Handle = I_Texture.GetHandle();
        VISERA_ASSERT(Handle != FRHITextureHandle{});
        RecordCommand(ECommandType::ConvertImageLayout, FConvertImageLayout
        {
            .Image     = Handle,
            .NewLayout = I_NewLayout,
        });
    }

    void FRHICommandList::
    CopyBufferToImage(const FRHIBufferID& I_Buffer, const FRHITextureID& I_Texture)
    {
        const auto BufferHandle = I_Buffer.GetHandle();
        const auto ImageHandle  = I_Texture.GetHandle();
        VISERA_ASSERT(BufferHandle != FRHIBufferHandle{});
        VISERA_ASSERT(ImageHandle  != FRHITextureHandle{});
        RecordCommand(ECommandType::CopyBufferToImage, FCopyBufferToImage
        {
            .Buffer = BufferHandle,
            .Image  = ImageHandle,
        });
    }

    void FRHICommandList::
    SetViewport(const FRHIViewport& I_Viewport)
    {
        RecordCommand(ECommandType::SetViewport, FSetViewport
        {
            .Viewport = I_Viewport,
        });
    }

    void FRHICommandList::
    SetScissor(const FRHIScissor& I_Scissor)
    {
        RecordCommand(ECommandType::SetScissor, FSetScissor
        {
            .Scissor = I_Scissor,
        });
    }

    void FRHICommandList::
    ClearColorImage(const FRHITextureID& I_Texture, FRHIClearColor I_ClearColor)
    {
        const auto Handle = I_Texture.GetHandle();
        VISERA_ASSERT(Handle != FRHITextureHandle{});
        RecordCommand(ECommandType::ClearColorImage, FClearColorImage
        {
            .Image      = Handle,
            .ClearColor = I_ClearColor,
        });
    }

    void FRHICommandList::
    BlitImage(const FRHITextureID& I_SrcTexture, const FRHITextureID& I_DstTexture, ERHIFilter I_Filter)
    {
        const auto SrcHandle = I_SrcTexture.GetHandle();
        const auto DstHandle = I_DstTexture.GetHandle();
        VISERA_ASSERT(SrcHandle != FRHITextureHandle{});
        VISERA_ASSERT(DstHandle != FRHITextureHandle{});
        RecordCommand(ECommandType::BlitImage, FBlitImage
        {
            .SrcImage = SrcHandle,
            .DstImage = DstHandle,
            .Filter   = I_Filter,
        });
    }

    void FRHICommandList::
    BlitToSwapChain(const FRHITextureID& I_Texture, ERHIFilter I_Filter)
    {
        const auto Handle = I_Texture.GetHandle();
        VISERA_ASSERT(Handle != FRHITextureHandle{});
        RecordCommand(ECommandType::BlitToSwapChain, FBlitToSwapChain
        {
            .Image  = Handle,
            .Filter = I_Filter,
        });
    }

    void FRHICommandList::
    EnterRenderPass(const FRHIRenderPassID& I_RenderPass)
    {
        const auto Handle = I_RenderPass.GetHandle();
        VISERA_ASSERT(Handle != FRHIRenderPassHandle{});
        RecordCommand(ECommandType::EnterRenderPass, FEnterRenderPass
        {
            .RenderPass = Handle,
        });
    }

    void FRHICommandList::
    LeaveRenderPass()
    {
        RecordCommand(ECommandType::LeaveRenderPass, FLeaveRenderPass{});
    }

    void FRHICommandList::
    BindVertexBuffer(const FRHIBufferID& I_Buffer, UInt8 I_Binding, UInt64 I_Offset)
    {
        const auto Handle = I_Buffer.GetHandle();
        VISERA_ASSERT(Handle != FRHIBufferHandle{});
        RecordCommand(ECommandType::BindVertexBuffer, FBindVertexBuffer
        {
            .Buffer   = Handle,
            .Binding  = I_Binding,
            .Offset   = I_Offset,
        });
    }

    void FRHICommandList::
    BindDescriptorSet(const FRHIDescriptorSetID& I_DescriptorSet, UInt32 I_SetIndex)
    {
        const auto Handle = I_DescriptorSet.GetHandle();
        VISERA_ASSERT(Handle != FRHIDescriptorSetHandle{});
        RecordCommand(ECommandType::BindDescriptorSet, FBindDescriptorSet
        {
            .DescriptorSet = Handle,
            .SetIndex      = I_SetIndex,
        });
    }

    void FRHICommandList::
    Draw(UInt32 I_VertexCount, UInt32 I_InstanceCount, UInt32 I_FirstVertex, UInt32 I_FirstInstance)
    {
        RecordCommand(ECommandType::Draw, FDraw
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
        RecordCommand(ECommandType::DrawIndexed, FDrawIndexed
        {
            .IndexCount    = I_IndexCount,
            .InstanceCount = I_InstanceCount,
            .FirstIndex    = I_FirstIndex,
            .VertexOffset  = I_VertexOffset,
            .FirstInstance = I_FirstInstance,
        });
    }
}
VISERA_MAKE_FORMATTER(Visera::ECommandType,
    const char* CommandName = "Unknown";
    switch (I_Formatee)
    {
    case Visera::ECommandType::ConvertImageLayout:  CommandName = "\"ConvertImageLayout\""; break;
    case Visera::ECommandType::WriteBuffer:         CommandName = "\"WriteBuffer\""; break;
    case Visera::ECommandType::CopyBufferToImage:   CommandName = "\"CopyBufferToImage\""; break;
    case Visera::ECommandType::ClearColorImage:     CommandName = "\"ClearColorImage\""; break;
    case Visera::ECommandType::BlitImage:           CommandName = "\"BlitImage\""; break;
    case Visera::ECommandType::BlitToSwapChain:     CommandName = "\"BlitToSwapChain\""; break;
    case Visera::ECommandType::EnterRenderPass:     CommandName = "\"EnterRenderPass\""; break;
    case Visera::ECommandType::SetViewport:        CommandName = "\"SetViewport\""; break;
    case Visera::ECommandType::SetScissor:         CommandName = "\"SetScissor\""; break;
    case Visera::ECommandType::LeaveRenderPass:    CommandName = "\"LeaveRenderPass\""; break;
    case Visera::ECommandType::BindVertexBuffer:   CommandName = "\"BindVertexBuffer\""; break;
    case Visera::ECommandType::BindDescriptorSet:  CommandName = "\"BindDescriptorSet\""; break;
    case Visera::ECommandType::Draw:               CommandName = "\"Draw\""; break;
    case Visera::ECommandType::DrawIndexed:        CommandName = "\"DrawIndexed\""; break;
    default: break;
    }
, "{}", CommandName);