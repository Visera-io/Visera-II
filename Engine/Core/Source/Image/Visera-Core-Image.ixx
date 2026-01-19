module;
#include <Visera-Core.hpp>
#define STB_IMAGE_RESIZE2_IMPLEMENTATION
#include <stb_image_resize2.h>
export module Visera.Core.Image;
#define VISERA_MODULE_NAME "Core.Image"
export import Visera.Core.Image.Pixel;
export import Visera.Core.Image.Common;
       import Visera.Core.Color;
       import Visera.Core.Types.Array;
       import Visera.Core.OS.Memory;

export namespace Visera
{
    /**
     * Minimal image data container for raw pixel storage.
     */
    class VISERA_CORE_API FImage
    {
    public:
        struct FCreateInfo
        {
            UInt32          Width          {0};
            UInt32          Height         {0};
            UInt32          Depth          {1};
            EPixelFormat    PixelFormat    {EPixelFormat::RGBA8_UNorm};
            EColorSpace     ColorSpace     {EColorSpace::Linear};
            UInt32          RowPitchBytes  {0}; // Auto-calculated if 0 (tight packing)
            UInt32          SlicePitchBytes{0}; // Auto-calculated if 0 (RowPitchBytes * Height)

            std::pmr::memory_resource*
            MemoryArena = std::pmr::get_default_resource();
        };

        [[nodiscard]] inline FByte*
        AccessData() { return Data.Data(); }
        [[nodiscard]] inline const FByte*
        GetData() const { return Data.Data(); }
        [[nodiscard]] inline UInt32
        GetWidth() const { return Info.Width; }
        [[nodiscard]] inline UInt32
        GetHeight() const { return Info.Height; }
        [[nodiscard]] inline UInt32
        GetDepth() const { return Info.Depth; }
        [[nodiscard]] inline EPixelFormat
        GetPixelFormat() const { return Info.PixelFormat; }
        [[nodiscard]] inline EColorSpace
        GetColorSpace() const { return Info.ColorSpace; }
        [[nodiscard]] inline UInt32
        GetRowPitchBytes() const { return Info.RowPitchBytes > 0? Info.RowPitchBytes : Info.Width * FPixel::GetByteSize(Info.PixelFormat); }
        [[nodiscard]] inline UInt32
        GetSlicePitchBytes() const { return Info.SlicePitchBytes > 0? Info.SlicePitchBytes : GetRowPitchBytes() * Info.Height; }
        [[nodiscard]] inline UInt8
        GetBytesPerPixel() const { return FPixel::GetByteSize(Info.PixelFormat); }
        [[nodiscard]] inline UInt8
        GetChannelCount() const { return FPixel::GetChannelCount(Info.PixelFormat); }
        [[nodiscard]] inline UInt64
        GetSizeInBytes() const { return static_cast<UInt64>(GetSlicePitchBytes()) * Info.Depth; }

        [[nodiscard]] Bool
        Resize(UInt32 I_NewWidth, UInt32 I_NewHeight);

        [[nodiscard]] inline Bool
        IsSRGB() const { return Info.ColorSpace == EColorSpace::sRGB; }
        [[nodiscard]] inline Bool
        IsRGBA() const { return Info.PixelFormat == EPixelFormat::RGBA8_UNorm || Info.PixelFormat == EPixelFormat::RGBA16_UNorm || Info.PixelFormat == EPixelFormat::RGBA16_Float || Info.PixelFormat == EPixelFormat::RGBA32_Float; }
        [[nodiscard]] inline Bool
        IsBGRA() const { return Info.PixelFormat == EPixelFormat::BGRA8_UNorm; }
        [[nodiscard]] inline Bool
        HasAlpha() const { return FPixel::HasAlpha(Info.PixelFormat); }
        [[nodiscard]] inline Bool
        IsFloatFormat() const { return FPixel::IsFloatFormat(Info.PixelFormat); }

    private:
        TPMRArray<FByte> Data;
        FCreateInfo      Info;

    public:
        FImage()  = default;
        ~FImage() = default;
        FImage(const FCreateInfo& I_CreateInfo);

    private:
        [[nodiscard]] static inline stbir_pixel_layout
        MapPixelFormatToSTBIR(EPixelFormat I_Format) noexcept;
    };

    FImage::
    FImage(const FCreateInfo& I_CreateInfo)
    : Data  (I_CreateInfo.MemoryArena),
      Info  (I_CreateInfo)
    {
        VISERA_ASSERT(I_CreateInfo.PixelFormat != EPixelFormat::Invalid);
        VISERA_ASSERT(I_CreateInfo.Width > 0 && I_CreateInfo.Height > 0 && I_CreateInfo.Depth > 0);
        VISERA_ASSERT(FPixel::GetByteSize(I_CreateInfo.PixelFormat) > 0);

        // Normalize pitches
        if (Info.RowPitchBytes == 0)
        {
            Info.RowPitchBytes = Info.Width * FPixel::GetByteSize(Info.PixelFormat);
        }
        if (Info.SlicePitchBytes == 0)
        {
            Info.SlicePitchBytes = Info.RowPitchBytes * Info.Height;
        }

        const auto SizeInBytes = GetSizeInBytes();
        if (SizeInBytes > 0)
        {
            Data.Resize(SizeInBytes);
        }
    }

    /**
     * Iterator for FImage that allows pixel-by-pixel access.
     * This iterator provides a view over the image data as pixels.
     */
    class VISERA_CORE_API FImagePixelIterator
    {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = FPixel;
        using difference_type = Int64;
        using pointer = FPixel*;
        using reference = FPixel&;

        FImagePixelIterator() = default;
        explicit FImagePixelIterator(FImage* I_Image, UInt64 I_Index = 0)
            : Image{I_Image}
            , PixelIndex{I_Index}
        {
        }

        [[nodiscard]] FPixel
        operator*() const
        {
            if (!Image || PixelIndex >= GetPixelCount())
            { return FPixel{nullptr, EPixelFormat::Invalid, 0}; }
            const UInt32 BytesPerPixel = Image->GetBytesPerPixel();
            const UInt32 RowPitch = Image->GetRowPitchBytes();
            const UInt32 X = static_cast<UInt32>(PixelIndex % Image->GetWidth());
            const UInt32 Y = static_cast<UInt32>(PixelIndex / Image->GetWidth());
            FByte* PixelData = Image->AccessData() + (Y * RowPitch + X * BytesPerPixel);
            return FPixel{PixelData, Image->GetPixelFormat(), static_cast<UInt8>(BytesPerPixel)};
        }

        FImagePixelIterator&
        operator++()
        {
            ++PixelIndex;
            return *this;
        }

        FImagePixelIterator
        operator++(Int32)
        {
            FImagePixelIterator Temp = *this;
            ++(*this);
            return Temp;
        }

        FImagePixelIterator&
        operator--()
        {
            --PixelIndex;
            return *this;
        }

        FImagePixelIterator
        operator--(Int32)
        {
            FImagePixelIterator Temp = *this;
            --(*this);
            return Temp;
        }

        FImagePixelIterator&
        operator+=(difference_type I_N)
        {
            PixelIndex += I_N;
            return *this;
        }

        FImagePixelIterator&
        operator-=(difference_type I_N)
        {
            PixelIndex -= I_N;
            return *this;
        }

        [[nodiscard]] FImagePixelIterator
        operator+(difference_type I_N) const
        {
            FImagePixelIterator Temp = *this;
            Temp += I_N;
            return Temp;
        }

        [[nodiscard]] FImagePixelIterator
        operator-(difference_type I_N) const
        {
            FImagePixelIterator Temp = *this;
            Temp -= I_N;
            return Temp;
        }

        [[nodiscard]] difference_type
        operator-(const FImagePixelIterator& I_Other) const
        {
            return static_cast<difference_type>(PixelIndex) - static_cast<difference_type>(I_Other.PixelIndex);
        }

        [[nodiscard]] Bool
        operator==(const FImagePixelIterator& I_Other) const
        {
            return Image == I_Other.Image && PixelIndex == I_Other.PixelIndex;
        }

        [[nodiscard]] Bool
        operator!=(const FImagePixelIterator& I_Other) const
        {
            return !(*this == I_Other);
        }

        [[nodiscard]] Bool
        operator<(const FImagePixelIterator& I_Other) const
        {
            return PixelIndex < I_Other.PixelIndex;
        }

        [[nodiscard]] Bool
        operator>(const FImagePixelIterator& I_Other) const
        {
            return PixelIndex > I_Other.PixelIndex;
        }

        [[nodiscard]] Bool
        operator<=(const FImagePixelIterator& I_Other) const
        {
            return PixelIndex <= I_Other.PixelIndex;
        }

        [[nodiscard]] Bool
        operator>=(const FImagePixelIterator& I_Other) const
        {
            return PixelIndex >= I_Other.PixelIndex;
        }

    private:
        FImage* Image = nullptr;
        UInt64 PixelIndex = 0;

    private:
        [[nodiscard]] UInt64
        GetPixelCount() const
        {
            if (!Image)
            { return 0; }
            return static_cast<UInt64>(Image->GetWidth()) * static_cast<UInt64>(Image->GetHeight()) * static_cast<UInt64>(Image->GetDepth());
        }
    };

    // Non-member operators
    [[nodiscard]] inline FImagePixelIterator
    operator+(FImagePixelIterator::difference_type I_N, const FImagePixelIterator& I_It)
    {
        return I_It + I_N;
    }

    // Add iterator support to FImage
    inline FImagePixelIterator
    begin(FImage& I_Image)
    {
        return FImagePixelIterator{&I_Image, 0};
    }

    inline FImagePixelIterator
    end(FImage& I_Image)
    {
        return FImagePixelIterator{&I_Image, static_cast<UInt64>(I_Image.GetWidth()) * static_cast<UInt64>(I_Image.GetHeight()) * static_cast<UInt64>(I_Image.GetDepth())};
    }

    /**
     * Maps EPixelFormat to stbir_pixel_layout for use with stb_image_resize2.
     * @param I_Format The pixel format to map
     * @return The corresponding stbir_pixel_layout value
     */
    stbir_pixel_layout FImage::
    MapPixelFormatToSTBIR(EPixelFormat I_Format) noexcept
    {
        switch (I_Format)
        {
        case EPixelFormat::R8_UNorm:
        case EPixelFormat::R16_UNorm:
        case EPixelFormat::R16_Float:
        case EPixelFormat::R32_Float:
            return STBIR_1CHANNEL;

        case EPixelFormat::RG8_UNorm:
        case EPixelFormat::RG16_UNorm:
        case EPixelFormat::RG16_Float:
        case EPixelFormat::RG32_Float:
            return STBIR_2CHANNEL;

        case EPixelFormat::RGB8_UNorm:
        case EPixelFormat::RGB16_UNorm:
        case EPixelFormat::RGB16_Float:
        case EPixelFormat::RGB32_Float:
        case EPixelFormat::RGBE8_HDR:
            return STBIR_RGB;

        case EPixelFormat::RGBA8_UNorm:
        case EPixelFormat::RGBA16_UNorm:
        case EPixelFormat::RGBA16_Float:
        case EPixelFormat::RGBA32_Float:
            return STBIR_RGBA;

        case EPixelFormat::BGRA8_UNorm:
            return STBIR_BGRA;

        case EPixelFormat::Invalid:
        default:
            VISERA_ASSERT(False);
            return STBIR_1CHANNEL;
        }
    }

    Bool FImage::
    Resize(UInt32 I_NewWidth, UInt32 I_NewHeight)
    {
        if (I_NewWidth == 0 || I_NewHeight == 0)
        {
            return False;
        }

        const auto PixelLayout = MapPixelFormatToSTBIR(Info.PixelFormat);
        const auto BytesPerPixel = GetBytesPerPixel();
        const auto InputRowPitch = GetRowPitchBytes();
        const auto InputSlicePitch = GetSlicePitchBytes();
        const auto OutputRowPitch = I_NewWidth * BytesPerPixel;
        const auto OutputSlicePitch = OutputRowPitch * I_NewHeight;
        const auto OutputSizeInBytes = static_cast<UInt64>(OutputSlicePitch) * Info.Depth;

        // Allocate output buffer
        TPMRArray<FByte> OutputBuffer(Info.MemoryArena);
        OutputBuffer.Resize(static_cast<TPMRArray<FByte>::SizeType>(OutputSizeInBytes));

        Bool bSuccess = True;

        // Process each depth slice
        for (UInt32 DepthSlice = 0; DepthSlice < Info.Depth; ++DepthSlice)
        {
            const auto* InputSliceData = Data.Data() + (DepthSlice * InputSlicePitch);
            auto* OutputSliceData = OutputBuffer.Data() + (DepthSlice * OutputSlicePitch);

            Bool bSliceSuccess = False;

            if (IsFloatFormat())
            {
                // For float formats, we need to handle both 16-bit and 32-bit floats
                // stb_image_resize2's float functions expect 32-bit floats
                if (Info.PixelFormat == EPixelFormat::R16_Float   ||
                    Info.PixelFormat == EPixelFormat::RG16_Float  ||
                    Info.PixelFormat == EPixelFormat::RGB16_Float ||
                    Info.PixelFormat == EPixelFormat::RGBA16_Float)
                {
                    // Convert 16-bit float to 32-bit float for processing
                    const auto ChannelCount = GetChannelCount();
                    const auto InputPixelCount = Info.Width * Info.Height;
                    const auto OutputPixelCount = I_NewWidth * I_NewHeight;

                    TArray<Float> InputFloatBuffer(InputPixelCount * ChannelCount);
                    TArray<Float> OutputFloatBuffer(OutputPixelCount * ChannelCount);

                    // Convert input from 16-bit float to 32-bit float
                    const auto* InputHalf = reinterpret_cast<const UInt16*>(InputSliceData);
                    for (UInt32 i = 0; i < InputPixelCount * ChannelCount; ++i)
                    {
                        // Half-to-float conversion (IEEE 754 binary16 to binary32)
                        const UInt16 Half = InputHalf[i];
                        const UInt32 Sign = (Half >> 15) & 0x1;
                        const UInt32 Exp = (Half >> 10) & 0x1F;
                        const UInt32 Mantissa = Half & 0x3FF;

                        Float FloatValue;
                        if (Exp == 0)
                        {
                            // Denormalized or zero
                            FloatValue = (Mantissa / 1024.0f) * (Sign ? -1.0f : 1.0f);
                        }
                        else if (Exp == 31)
                        {
                            // Infinity or NaN
                            FloatValue = (Mantissa == 0) ? (Sign ? -std::numeric_limits<Float>::infinity() : std::numeric_limits<Float>::infinity()) : std::numeric_limits<Float>::quiet_NaN();
                        }
                        else
                        {
                            // Normalized
                            const UInt32 FloatExp = Exp + 112; // Bias difference: 127 - 15
                            const UInt32 FloatMantissa = Mantissa << 13;
                            const UInt32 FloatBits = (Sign << 31) | (FloatExp << 23) | FloatMantissa;
                            std::memcpy(&FloatValue, &FloatBits, sizeof(Float));
                        }
                        InputFloatBuffer[i] = FloatValue;
                    }

                    // Resize using float function
                    bSliceSuccess = (stbir_resize_float_linear(
                        InputFloatBuffer.Data(), Info.Width, Info.Height, 0,
                        OutputFloatBuffer.Data(), I_NewWidth, I_NewHeight, 0,
                        PixelLayout) != nullptr);

                    if (bSliceSuccess)
                    {
                        // Convert output back from 32-bit float to 16-bit float
                        auto* OutputHalf = reinterpret_cast<UInt16*>(OutputSliceData);
                        for (UInt32 i = 0; i < OutputPixelCount * ChannelCount; ++i)
                        {
                            const Float FloatValue = OutputFloatBuffer[i];
                            // Float-to-half conversion (IEEE 754 binary32 to binary16)
                            UInt32 FloatBits;
                            std::memcpy(&FloatBits, &FloatValue, sizeof(Float));
                            const UInt32 Sign = (FloatBits >> 31) & 0x1;
                            const UInt32 Exp = (FloatBits >> 23) & 0xFF;
                            const UInt32 Mantissa = FloatBits & 0x7FFFFF;

                            UInt16 Half;
                            if (Exp == 0)
                            {
                                // Denormalized or zero
                                Half = static_cast<UInt16>((Sign << 15) | (Mantissa >> 13));
                            }
                            else if (Exp == 255)
                            {
                                // Infinity or NaN
                                Half = static_cast<UInt16>((Sign << 15) | 0x7C00 | (Mantissa ? 0x200 : 0));
                            }
                            else
                            {
                                const Int32 HalfExp = static_cast<Int32>(Exp) - 112; // Bias difference
                                if (HalfExp < 0)
                                {
                                    // Underflow to zero
                                    Half = static_cast<UInt16>(Sign << 15);
                                }
                                else if (HalfExp > 30)
                                {
                                    // Overflow to infinity
                                    Half = static_cast<UInt16>((Sign << 15) | 0x7C00);
                                }
                                else
                                {
                                    Half = static_cast<UInt16>((Sign << 15) | (HalfExp << 10) | (Mantissa >> 13));
                                }
                            }
                            OutputHalf[i] = Half;
                        }
                    }
                }
                else
                {
                    // 32-bit float formats - can use directly
                    bSliceSuccess = (stbir_resize_float_linear(
                        reinterpret_cast<const Float*>(InputSliceData), Info.Width, Info.Height, InputRowPitch,
                        reinterpret_cast<Float*>(OutputSliceData), I_NewWidth, I_NewHeight, OutputRowPitch,
                        PixelLayout) != nullptr);
                }
            }
            else if (Info.PixelFormat == EPixelFormat::R16_UNorm ||
                     Info.PixelFormat == EPixelFormat::RG16_UNorm ||
                     Info.PixelFormat == EPixelFormat::RGB16_UNorm ||
                     Info.PixelFormat == EPixelFormat::RGBA16_UNorm)
            {
                // 16-bit UNorm formats - convert to float, process, then convert back
                const auto ChannelCount = GetChannelCount();
                const auto InputPixelCount = Info.Width * Info.Height;
                const auto OutputPixelCount = I_NewWidth * I_NewHeight;

                TArray<Float> InputFloatBuffer(InputPixelCount * ChannelCount);
                TArray<Float> OutputFloatBuffer(OutputPixelCount * ChannelCount);

                // Convert input from 16-bit UNorm to float (normalize to 0.0-1.0)
                const auto* InputU16 = reinterpret_cast<const UInt16*>(InputSliceData);
                for (UInt32 i = 0; i < InputPixelCount * ChannelCount; ++i)
                {
                    InputFloatBuffer[i] = static_cast<Float>(InputU16[i]) / 65535.0f;
                }

                // Resize using float function
                bSliceSuccess = (stbir_resize_float_linear(
                    InputFloatBuffer.Data(), Info.Width, Info.Height, 0,
                    OutputFloatBuffer.Data(), I_NewWidth, I_NewHeight, 0,
                    PixelLayout) != nullptr);

                if (bSliceSuccess)
                {
                    // Convert output back from float to 16-bit UNorm
                    auto* OutputU16 = reinterpret_cast<UInt16*>(OutputSliceData);
                    for (UInt32 i = 0; i < OutputPixelCount * ChannelCount; ++i)
                    {
                        // Clamp to [0, 1] and convert to 16-bit
                        const Float ClampedValue = std::max(0.0f, std::min(1.0f, OutputFloatBuffer[i]));
                        OutputU16[i] = static_cast<UInt16>(ClampedValue * 65535.0f + 0.5f); // Round to nearest
                    }
                }
            }
            else
            {
                // Uint8 formats
                switch (Info.ColorSpace)
                {
                case EColorSpace::sRGB:
                    bSliceSuccess = (stbir_resize_uint8_srgb(
                        InputSliceData, Info.Width, Info.Height, InputRowPitch,
                        OutputSliceData, I_NewWidth, I_NewHeight, OutputRowPitch,
                        PixelLayout) != nullptr);
                    break;
                case EColorSpace::Linear:
                    bSliceSuccess = (stbir_resize_uint8_linear(
                        InputSliceData, Info.Width, Info.Height, InputRowPitch,
                        OutputSliceData, I_NewWidth, I_NewHeight, OutputRowPitch,
                        PixelLayout) != nullptr);
                    break;
                default:
                    VISERA_ASSERT(False);
                    bSliceSuccess = False;
                    break;
                }
            }

            if (!bSliceSuccess)
            {
                bSuccess = False;
                break;
            }
        }

        if (bSuccess)
        {
            // Update image info and swap buffers
            Info.Width = I_NewWidth;
            Info.Height = I_NewHeight;
            Info.RowPitchBytes = OutputRowPitch;
            Info.SlicePitchBytes = OutputSlicePitch;
            Data = std::move(OutputBuffer);
        }

        return bSuccess;
    }
}