module;
#include <Visera-RHI.hpp>
#include <cstring>
export module Visera.RHI.CommandList;
#define VISERA_MODULE_NAME "RHI.CommandList"
       import Visera.RHI.Common;
       import Visera.Global.Log;
       import Visera.Core.OS.Memory;
       import Visera.Core.Types.Array;

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
    };

    // Command view returned by iterator
    struct FCommandView
    {
        ECommandType Type;
        const FByte* PayloadPtrAligned; // Already aligned payload start
        UInt16       PayloadBytes;      // = TotalBytes - PayloadOff
    };

    class VISERA_RHI_API FRHICommandList
    {
        static constexpr UInt64 CommandAlignment = 8;

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
            FRHIBufferHandle Buffer;
            const FByte*     Data;
            UInt64           Size;
        };
        void inline
        WriteBuffer(FRHIBufferHandle I_Buffer, const FByte* I_Data, UInt64 I_Size);

        struct FEnterRenderPass
        {

        };
        void inline
        EnterRenderPass(FRHIRenderPassHandle I_RenderPass) {}

        struct FLeaveRenderPass
        {

        };
        void inline
        LeaveRenderPass() {}

        struct FCopyBufferToImage
        {
            FRHIBufferHandle  Buffer;
            FRHITextureHandle Image;
        };
        void inline
        CopyBufferToImage(FRHIBufferHandle I_Buffer, FRHITextureHandle I_Texture);

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
            FRHITextureHandle Image;
            ERHIImageLayout   NewLayout;
        };
        void inline
        ConvertImageLayout(FRHITextureHandle I_Texture, ERHIImageLayout I_NewLayout);

        struct FClearColorImage
        {
            FRHITextureHandle Image;
            FRHIClearColor    ClearColor;
        };
        void inline
        ClearColorImage(FRHITextureHandle I_Texture, FRHIClearColor I_ClearColor);

        struct FBlitImage
        {
            FRHITextureHandle SrcImage;
            FRHITextureHandle DstImage;
            ERHIFilter        Filter;
        };
        void inline
        BlitImage(FRHITextureHandle I_SrcTexture, FRHITextureHandle I_DstTexture, ERHIFilter I_Filter);

        struct FBlitToSwapChain
        {
            FRHITextureHandle Image;
            ERHIFilter        Filter;
        };
        void inline
        BlitToSwapChain(FRHITextureHandle I_Texture, ERHIFilter I_Filter);

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
        Reset() { Buffer.Clear(); MemoryCache.Reset(); CommandCount = 0; }

        // Iterator for range-based for loop
        class VISERA_RHI_API FIterator
        {
        public:
            FIterator(const FByte* I_Data, UInt64 I_Size, UInt64 I_Offset)
                : Data(I_Data)
                , Size(I_Size)
                , Offset(I_Offset)
            {
            }

            [[nodiscard]] FCommandView operator*() const
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
        Memory::TMonotonicArena<32_KB> MemoryCache;
        TPMRArray<FByte>               Buffer;
        UInt64                         CommandCount = 0;

    public:
        FRHICommandList() : MemoryCache(), Buffer(&MemoryCache.Get()) { }

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

#if !defined(VISERA_RELEASE_MODE)
            // Zero padding for debug-friendly buffer dumps
            if (PayloadEnd < CommandEnd)
            {
                Memory::Memset(Buffer.Data() + PayloadEnd, 0, CommandEnd - PayloadEnd);
            }
#endif

            ++CommandCount;
        }
    };

    void FRHICommandList::
    WriteBuffer(FRHIBufferHandle I_Buffer, const FByte* I_Data, UInt64 I_Size)
    {
        VISERA_ASSERT(I_Buffer != FRHIBufferHandle{});
        RecordCommand(ECommandType::WriteBuffer, FWriteBuffer
        {
            .Buffer = I_Buffer,
            .Data   = I_Data,
            .Size   = I_Size,
        });
    }

    void FRHICommandList::
    ConvertImageLayout(FRHITextureHandle I_Texture, ERHIImageLayout I_NewLayout)
    {
        VISERA_ASSERT(I_Texture != FRHITextureHandle{});
        RecordCommand(ECommandType::ConvertImageLayout, FConvertImageLayout
        {
            .Image     = I_Texture,
            .NewLayout = I_NewLayout,
        });
    }

    void FRHICommandList::
    CopyBufferToImage(FRHIBufferHandle I_Buffer, FRHITextureHandle I_Texture)
    {
        VISERA_ASSERT(I_Buffer  != FRHIBufferHandle{});
        VISERA_ASSERT(I_Texture != FRHITextureHandle{});
        RecordCommand(ECommandType::CopyBufferToImage, FCopyBufferToImage
        {
            .Buffer = I_Buffer,
            .Image  = I_Texture,
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
    ClearColorImage(FRHITextureHandle I_Texture, FRHIClearColor I_ClearColor)
    {
        VISERA_ASSERT(I_Texture != FRHITextureHandle{});
        RecordCommand(ECommandType::ClearColorImage, FClearColorImage
        {
            .Image      = I_Texture,
            .ClearColor = I_ClearColor,
        });
    }

    void FRHICommandList::
    BlitImage(FRHITextureHandle I_SrcTexture,
              FRHITextureHandle I_DstTexture,
              ERHIFilter        I_Filter)
    {
        VISERA_ASSERT(I_SrcTexture != FRHITextureHandle{});
        VISERA_ASSERT(I_DstTexture != FRHITextureHandle{});
        RecordCommand(ECommandType::BlitImage, FBlitImage
        {
            .SrcImage = I_SrcTexture,
            .DstImage = I_DstTexture,
            .Filter   = I_Filter,
        });
    }

    void FRHICommandList::
    BlitToSwapChain(FRHITextureHandle I_Texture, ERHIFilter I_Filter)
    {
        VISERA_ASSERT(I_Texture != FRHITextureHandle{});
        RecordCommand(ECommandType::BlitToSwapChain, FBlitToSwapChain
        {
            .Image  = I_Texture,
            .Filter = I_Filter,
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
    case Visera::ECommandType::LeaveRenderPass:     CommandName = "\"LeaveRenderPass\""; break;
    default: break;
    }
, "{}", CommandName);