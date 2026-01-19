module;
#include <Visera-Core.hpp>
export module Visera.Core.Image.Pixel;
#define VISERA_MODULE_NAME "Core.Image"
export import Visera.Core.Image.Common;
       import Visera.Core.Color;
       import Visera.Core.Types.Half;
       import Visera.Core.Math.Arithmetic.Operation;

export namespace Visera
{
    /**
     * Pixel accessor/view for FImage, similar to FCommandView in RHI.
     * Provides read/write access to a single pixel in an image.
     * Also provides static utility methods for pixel format operations.
     */
    class VISERA_CORE_API FPixel
    {
    public:
        /**
         * Calculates the number of bytes per pixel for a given pixel format.
         * @param I_Format The pixel format
         * @return The number of bytes per pixel
         */
        [[nodiscard]] static constexpr UInt8
        GetByteSize(EPixelFormat I_Format) noexcept;
        [[nodiscard]] static constexpr UInt8
        GetChannelCount(EPixelFormat I_Format) noexcept;
        [[nodiscard]] static constexpr Bool
        HasAlpha(EPixelFormat I_Format) noexcept;
        [[nodiscard]] static constexpr Bool
        IsFloatFormat(EPixelFormat I_Format) noexcept;

        /**
         * Gets the pointer to pixel data.
         * @return Pointer to pixel data
         */
        [[nodiscard]] FByte* GetData() { return Data; }

        /**
         * Gets the const pointer to pixel data.
         * @return Const pointer to pixel data
         */
        [[nodiscard]] const FByte* GetData() const { return Data; }

        /**
         * Gets the pixel format.
         * @return Pixel format
         */
        [[nodiscard]] EPixelFormat GetPixelFormat() const { return PixelFormat; }

        /**
         * Gets the bytes per pixel.
         * @return Bytes per pixel
         */
        [[nodiscard]] UInt8 GetBytesPerPixel() const { return BytesPerPixel; }

        /**
         * Gets the pixel value as FColor.
         * This is a RAW READ operation - it directly reads the pixel values
         * from the pixel format without any colorspace conversion (e.g., sRGB).
         * @return FColor value representing the pixel
         */
        [[nodiscard]] FColor Get() const;

        /**
         * Sets the pixel value from a color type.
         * This is a RAW WRITE operation - it directly writes the color values
         * to the pixel format without any colorspace conversion (e.g., sRGB).
         * For colorspace-aware writes, use FImage-level methods that handle sRGB conversion.
         * @tparam TColor Color type (must satisfy Concepts::Color, defaults to FColor)
         * @param I_Color Color value
         */
        template<Concepts::Color TColor = FColor>
        void Set(const TColor& I_Color);

    private:
        FByte*       Data;          // Pointer to pixel data
        EPixelFormat PixelFormat;   // Format of the pixel
        UInt8        BytesPerPixel; // Bytes per pixel (cached for performance)

    public:
        /**
         * Constructs a read-only pixel view from const data pointer.
         * @param I_Data Const pointer to pixel data
         * @param I_PixelFormat Format of the pixel
         * @param I_BytesPerPixel Bytes per pixel (cached for performance)
         */
        explicit FPixel(const FByte* I_Data, EPixelFormat I_PixelFormat, UInt8 I_BytesPerPixel)
            : Data{const_cast<FByte*>(I_Data)}
            , PixelFormat{I_PixelFormat}
            , BytesPerPixel{I_BytesPerPixel}
        {
        }
    };

    /**
     * Calculates the number of bytes per pixel for a given pixel format.
     * @param I_Format The pixel format
     * @return The number of bytes per pixel
     */
    [[nodiscard]] constexpr UInt8 FPixel::
    GetByteSize(EPixelFormat I_Format) noexcept
    {
        switch (I_Format)
        {
        // 8-bit formats
        case EPixelFormat::R8_UNorm:        return 1;
        case EPixelFormat::RG8_UNorm:       return 2;
        case EPixelFormat::RGB8_UNorm:      return 3;
        case EPixelFormat::RGBA8_UNorm:     return 4;
        case EPixelFormat::BGRA8_UNorm:     return 4;

        // 16-bit formats
        case EPixelFormat::R16_UNorm:       return 2;
        case EPixelFormat::RG16_UNorm:      return 4;
        case EPixelFormat::RGB16_UNorm:     return 6;
        case EPixelFormat::RGBA16_UNorm:    return 8;

        // 16-bit float formats
        case EPixelFormat::R16_Float:       return 2;
        case EPixelFormat::RG16_Float:      return 4;
        case EPixelFormat::RGB16_Float:     return 6;
        case EPixelFormat::RGBA16_Float:    return 8;

        // 32-bit float formats
        case EPixelFormat::R32_Float:       return 4;
        case EPixelFormat::RG32_Float:      return 8;
        case EPixelFormat::RGB32_Float:     return 12;
        case EPixelFormat::RGBA32_Float:    return 16;

        // Special formats
        case EPixelFormat::RGBE8_HDR:       return 4; // RGBE: 3x8-bit RGB + 8-bit exponent

        case EPixelFormat::Invalid:
        default: return 0;
        }
    }

    /**
     * Gets the number of channels for a given pixel format.
     * @param I_Format The pixel format
     * @return The number of channels (1-4)
     */
    [[nodiscard]] constexpr UInt8 FPixel::
    GetChannelCount(EPixelFormat I_Format) noexcept
    {
        switch (I_Format)
        {
        case EPixelFormat::R8_UNorm:
        case EPixelFormat::R16_UNorm:
        case EPixelFormat::R16_Float:
        case EPixelFormat::R32_Float:
            return 1;

        case EPixelFormat::RG8_UNorm:
        case EPixelFormat::RG16_UNorm:
        case EPixelFormat::RG16_Float:
        case EPixelFormat::RG32_Float:
            return 2;

        case EPixelFormat::RGB8_UNorm:
        case EPixelFormat::RGB16_UNorm:
        case EPixelFormat::RGB16_Float:
        case EPixelFormat::RGB32_Float:
        case EPixelFormat::RGBE8_HDR:
            return 3;

        case EPixelFormat::RGBA8_UNorm:
        case EPixelFormat::BGRA8_UNorm:
        case EPixelFormat::RGBA16_UNorm:
        case EPixelFormat::RGBA16_Float:
        case EPixelFormat::RGBA32_Float:
            return 4;

        case EPixelFormat::Invalid:
        default: return 0;
        }
    }

    /**
     * Checks if a pixel format has an alpha channel.
     * @param I_Format The pixel format
     * @return True if the format has an alpha channel
     */
    [[nodiscard]] constexpr Bool FPixel::
    HasAlpha(EPixelFormat I_Format) noexcept
    {
        return I_Format == EPixelFormat::RGBA8_UNorm    ||
               I_Format == EPixelFormat::BGRA8_UNorm    ||
               I_Format == EPixelFormat::RGBA16_UNorm   ||
               I_Format == EPixelFormat::RGBA16_Float   ||
               I_Format == EPixelFormat::RGBA32_Float;
    }

    /**
     * Checks if a pixel format uses floating-point representation.
     * @param I_Format The pixel format
     * @return True if the format uses floats
     */
    [[nodiscard]] constexpr Bool FPixel::
    IsFloatFormat(EPixelFormat I_Format) noexcept
    {
        return I_Format == EPixelFormat::R16_Float      ||
               I_Format == EPixelFormat::RG16_Float     ||
               I_Format == EPixelFormat::RGB16_Float    ||
               I_Format == EPixelFormat::RGBA16_Float   ||
               I_Format == EPixelFormat::R32_Float      ||
               I_Format == EPixelFormat::RG32_Float     ||
               I_Format == EPixelFormat::RGB32_Float    ||
               I_Format == EPixelFormat::RGBA32_Float;
    }

    FColor FPixel::
    Get() const
    {
        if (!Data) { return FColor{}; }

        FColor Result{};

        switch (PixelFormat)
        {
        // 8-bit UNorm formats
        case EPixelFormat::R8_UNorm:
        {
            const UInt8* U8Data = reinterpret_cast<const UInt8*>(Data);
            Result.R = U8Data[0];
            Result.G = 0;
            Result.B = 0;
            Result.A = 255;
            break;
        }
        case EPixelFormat::RG8_UNorm:
        {
            const UInt8* U8Data = reinterpret_cast<const UInt8*>(Data);
            Result.R = U8Data[0];
            Result.G = U8Data[1];
            Result.B = 0;
            Result.A = 255;
            break;
        }
        case EPixelFormat::RGB8_UNorm:
        {
            const UInt8* U8Data = reinterpret_cast<const UInt8*>(Data);
            Result.R = U8Data[0];
            Result.G = U8Data[1];
            Result.B = U8Data[2];
            Result.A = 255;
            break;
        }
        case EPixelFormat::RGBA8_UNorm:
        {
            const UInt8* U8Data = reinterpret_cast<const UInt8*>(Data);
            Result.R = U8Data[0];
            Result.G = U8Data[1];
            Result.B = U8Data[2];
            Result.A = U8Data[3];
            break;
        }
        case EPixelFormat::BGRA8_UNorm:
        {
            const UInt8* U8Data = reinterpret_cast<const UInt8*>(Data);
            Result.R = U8Data[2]; // BGR -> RGB
            Result.G = U8Data[1];
            Result.B = U8Data[0];
            Result.A = U8Data[3];
            break;
        }

        // 16-bit UNorm formats
        case EPixelFormat::R16_UNorm:
        {
            const UInt16* U16Data = reinterpret_cast<const UInt16*>(Data);
            Result.R = static_cast<UInt8>((U16Data[0] * 255) / 65535);
            Result.G = 0;
            Result.B = 0;
            Result.A = 255;
            break;
        }
        case EPixelFormat::RG16_UNorm:
        {
            const UInt16* U16Data = reinterpret_cast<const UInt16*>(Data);
            Result.R = static_cast<UInt8>((U16Data[0] * 255) / 65535);
            Result.G = static_cast<UInt8>((U16Data[1] * 255) / 65535);
            Result.B = 0;
            Result.A = 255;
            break;
        }
        case EPixelFormat::RGB16_UNorm:
        {
            const UInt16* U16Data = reinterpret_cast<const UInt16*>(Data);
            Result.R = static_cast<UInt8>((U16Data[0] * 255) / 65535);
            Result.G = static_cast<UInt8>((U16Data[1] * 255) / 65535);
            Result.B = static_cast<UInt8>((U16Data[2] * 255) / 65535);
            Result.A = 255;
            break;
        }
        case EPixelFormat::RGBA16_UNorm:
        {
            const UInt16* U16Data = reinterpret_cast<const UInt16*>(Data);
            Result.R = static_cast<UInt8>((U16Data[0] * 255) / 65535);
            Result.G = static_cast<UInt8>((U16Data[1] * 255) / 65535);
            Result.B = static_cast<UInt8>((U16Data[2] * 255) / 65535);
            Result.A = static_cast<UInt8>((U16Data[3] * 255) / 65535);
            break;
        }

        // 16-bit Float formats
        case EPixelFormat::R16_Float:
        {
            const UInt16* HalfBits = reinterpret_cast<const UInt16*>(Data);
            const FHalf HalfValue = FHalf::FromBits(HalfBits[0]);
            const Float FloatValue = static_cast<Float>(HalfValue);
            Result.R = static_cast<UInt8>(Math::Clamp(FloatValue, 0.0f, 1.0f) * 255.0f + 0.5f);
            Result.G = 0;
            Result.B = 0;
            Result.A = 255;
            break;
        }
        case EPixelFormat::RG16_Float:
        {
            const UInt16* HalfBits = reinterpret_cast<const UInt16*>(Data);
            const FHalf HalfR = FHalf::FromBits(HalfBits[0]);
            const FHalf HalfG = FHalf::FromBits(HalfBits[1]);
            Result.R = static_cast<UInt8>(Math::Clamp(static_cast<Float>(HalfR), 0.0f, 1.0f) * 255.0f + 0.5f);
            Result.G = static_cast<UInt8>(Math::Clamp(static_cast<Float>(HalfG), 0.0f, 1.0f) * 255.0f + 0.5f);
            Result.B = 0;
            Result.A = 255;
            break;
        }
        case EPixelFormat::RGB16_Float:
        {
            const UInt16* HalfBits = reinterpret_cast<const UInt16*>(Data);
            const FHalf HalfR = FHalf::FromBits(HalfBits[0]);
            const FHalf HalfG = FHalf::FromBits(HalfBits[1]);
            const FHalf HalfB = FHalf::FromBits(HalfBits[2]);
            Result.R = static_cast<UInt8>(Math::Clamp(static_cast<Float>(HalfR), 0.0f, 1.0f) * 255.0f + 0.5f);
            Result.G = static_cast<UInt8>(Math::Clamp(static_cast<Float>(HalfG), 0.0f, 1.0f) * 255.0f + 0.5f);
            Result.B = static_cast<UInt8>(Math::Clamp(static_cast<Float>(HalfB), 0.0f, 1.0f) * 255.0f + 0.5f);
            Result.A = 255;
            break;
        }
        case EPixelFormat::RGBA16_Float:
        {
            const UInt16* HalfBits = reinterpret_cast<const UInt16*>(Data);
            const FHalf HalfR = FHalf::FromBits(HalfBits[0]);
            const FHalf HalfG = FHalf::FromBits(HalfBits[1]);
            const FHalf HalfB = FHalf::FromBits(HalfBits[2]);
            const FHalf HalfA = FHalf::FromBits(HalfBits[3]);
            Result.R = static_cast<UInt8>(Math::Clamp(static_cast<Float>(HalfR), 0.0f, 1.0f) * 255.0f + 0.5f);
            Result.G = static_cast<UInt8>(Math::Clamp(static_cast<Float>(HalfG), 0.0f, 1.0f) * 255.0f + 0.5f);
            Result.B = static_cast<UInt8>(Math::Clamp(static_cast<Float>(HalfB), 0.0f, 1.0f) * 255.0f + 0.5f);
            Result.A = static_cast<UInt8>(Math::Clamp(static_cast<Float>(HalfA), 0.0f, 1.0f) * 255.0f + 0.5f);
            break;
        }

        // 32-bit Float formats
        case EPixelFormat::R32_Float:
        {
            const Float* FloatData = reinterpret_cast<const Float*>(Data);
            Result.R = static_cast<UInt8>(Math::Clamp(FloatData[0], 0.0f, 1.0f) * 255.0f + 0.5f);
            Result.G = 0;
            Result.B = 0;
            Result.A = 255;
            break;
        }
        case EPixelFormat::RG32_Float:
        {
            const Float* FloatData = reinterpret_cast<const Float*>(Data);
            Result.R = static_cast<UInt8>(Math::Clamp(FloatData[0], 0.0f, 1.0f) * 255.0f + 0.5f);
            Result.G = static_cast<UInt8>(Math::Clamp(FloatData[1], 0.0f, 1.0f) * 255.0f + 0.5f);
            Result.B = 0;
            Result.A = 255;
            break;
        }
        case EPixelFormat::RGB32_Float:
        {
            const Float* FloatData = reinterpret_cast<const Float*>(Data);
            Result.R = static_cast<UInt8>(Math::Clamp(FloatData[0], 0.0f, 1.0f) * 255.0f + 0.5f);
            Result.G = static_cast<UInt8>(Math::Clamp(FloatData[1], 0.0f, 1.0f) * 255.0f + 0.5f);
            Result.B = static_cast<UInt8>(Math::Clamp(FloatData[2], 0.0f, 1.0f) * 255.0f + 0.5f);
            Result.A = 255;
            break;
        }
        case EPixelFormat::RGBA32_Float:
        {
            const Float* FloatData = reinterpret_cast<const Float*>(Data);
            Result.R = static_cast<UInt8>(Math::Clamp(FloatData[0], 0.0f, 1.0f) * 255.0f + 0.5f);
            Result.G = static_cast<UInt8>(Math::Clamp(FloatData[1], 0.0f, 1.0f) * 255.0f + 0.5f);
            Result.B = static_cast<UInt8>(Math::Clamp(FloatData[2], 0.0f, 1.0f) * 255.0f + 0.5f);
            Result.A = static_cast<UInt8>(Math::Clamp(FloatData[3], 0.0f, 1.0f) * 255.0f + 0.5f);
            break;
        }

        // Special formats
        case EPixelFormat::RGBE8_HDR:
        {
            // RGBE format: decode from shared exponent representation
            const UInt8* U8Data = reinterpret_cast<const UInt8*>(Data);
            if (U8Data[3] == 0)
            {
                Result.R = 0;
                Result.G = 0;
                Result.B = 0;
                Result.A = 255;
            }
            else
            {
                const Int32 Exponent = static_cast<Int32>(U8Data[3]) - 128;
                const Float Scale = std::ldexp(1.0f, Exponent - 8); // 2^(exponent-8)
                const Float R = static_cast<Float>(U8Data[0]) * Scale;
                const Float G = static_cast<Float>(U8Data[1]) * Scale;
                const Float B = static_cast<Float>(U8Data[2]) * Scale;
                Result.R = static_cast<UInt8>(Math::Clamp(R, 0.0f, 1.0f) * 255.0f + 0.5f);
                Result.G = static_cast<UInt8>(Math::Clamp(G, 0.0f, 1.0f) * 255.0f + 0.5f);
                Result.B = static_cast<UInt8>(Math::Clamp(B, 0.0f, 1.0f) * 255.0f + 0.5f);
                Result.A = 255;
            }
            break;
        }

        case EPixelFormat::Invalid:
        default:
            // Unsupported format - return black
            Result = FColor::Black();
            break;
        }

        return Result;
    }

    template<Concepts::Color TColor = FColor> void FPixel::
    Set(const TColor& I_Color)
    {
        if (!Data) { return; }

        // Convert color components to Float for processing
        Float ClampedR, ClampedG, ClampedB, ClampedA;
        
        if constexpr (std::is_same_v<TColor, FColor>)
        {
            // FColor: UInt8 (0-255) -> Float (0.0-1.0)
            ClampedR = static_cast<Float>(I_Color.R) / 255.0f;
            ClampedG = static_cast<Float>(I_Color.G) / 255.0f;
            ClampedB = static_cast<Float>(I_Color.B) / 255.0f;
            ClampedA = static_cast<Float>(I_Color.A) / 255.0f;
        }
        else if constexpr (std::is_same_v<TColor, FLinearColor>)
        {
            // FLinearColor: already Float (0.0-1.0)
            ClampedR = Math::Clamp(I_Color.R, 0.0f, 1.0f);
            ClampedG = Math::Clamp(I_Color.G, 0.0f, 1.0f);
            ClampedB = Math::Clamp(I_Color.B, 0.0f, 1.0f);
            ClampedA = Math::Clamp(I_Color.A, 0.0f, 1.0f);
        }
        else
        {
            // Generic color type: assume Float components (0.0-1.0)
            ClampedR = Math::Clamp(static_cast<Float>(I_Color.R), 0.0f, 1.0f);
            ClampedG = Math::Clamp(static_cast<Float>(I_Color.G), 0.0f, 1.0f);
            ClampedB = Math::Clamp(static_cast<Float>(I_Color.B), 0.0f, 1.0f);
            ClampedA = Math::Clamp(static_cast<Float>(I_Color.A), 0.0f, 1.0f);
        }

        switch (PixelFormat)
        {
        // 8-bit UNorm formats
        case EPixelFormat::R8_UNorm:
        {
            UInt8* U8Data = reinterpret_cast<UInt8*>(Data);
            U8Data[0] = static_cast<UInt8>(ClampedR * 255.0f + 0.5f);
            break;
        }
        case EPixelFormat::RG8_UNorm:
        {
            UInt8* U8Data = reinterpret_cast<UInt8*>(Data);
            U8Data[0] = static_cast<UInt8>(ClampedR * 255.0f + 0.5f);
            U8Data[1] = static_cast<UInt8>(ClampedG * 255.0f + 0.5f);
            break;
        }
        case EPixelFormat::RGB8_UNorm:
        {
            UInt8* U8Data = reinterpret_cast<UInt8*>(Data);
            U8Data[0] = static_cast<UInt8>(ClampedR * 255.0f + 0.5f);
            U8Data[1] = static_cast<UInt8>(ClampedG * 255.0f + 0.5f);
            U8Data[2] = static_cast<UInt8>(ClampedB * 255.0f + 0.5f);
            break;
        }
        case EPixelFormat::RGBA8_UNorm:
        {
            UInt8* U8Data = reinterpret_cast<UInt8*>(Data);
            U8Data[0] = static_cast<UInt8>(ClampedR * 255.0f + 0.5f);
            U8Data[1] = static_cast<UInt8>(ClampedG * 255.0f + 0.5f);
            U8Data[2] = static_cast<UInt8>(ClampedB * 255.0f + 0.5f);
            U8Data[3] = static_cast<UInt8>(ClampedA * 255.0f + 0.5f);
            break;
        }
        case EPixelFormat::BGRA8_UNorm:
        {
            UInt8* U8Data = reinterpret_cast<UInt8*>(Data);
            U8Data[0] = static_cast<UInt8>(ClampedB * 255.0f + 0.5f); // B
            U8Data[1] = static_cast<UInt8>(ClampedG * 255.0f + 0.5f); // G
            U8Data[2] = static_cast<UInt8>(ClampedR * 255.0f + 0.5f); // R
            U8Data[3] = static_cast<UInt8>(ClampedA * 255.0f + 0.5f); // A
            break;
        }

        // 16-bit UNorm formats
        case EPixelFormat::R16_UNorm:
        {
            UInt16* U16Data = reinterpret_cast<UInt16*>(Data);
            U16Data[0] = static_cast<UInt16>(ClampedR * 65535.0f + 0.5f);
            break;
        }
        case EPixelFormat::RG16_UNorm:
        {
            UInt16* U16Data = reinterpret_cast<UInt16*>(Data);
            U16Data[0] = static_cast<UInt16>(ClampedR * 65535.0f + 0.5f);
            U16Data[1] = static_cast<UInt16>(ClampedG * 65535.0f + 0.5f);
            break;
        }
        case EPixelFormat::RGB16_UNorm:
        {
            UInt16* U16Data = reinterpret_cast<UInt16*>(Data);
            U16Data[0] = static_cast<UInt16>(ClampedR * 65535.0f + 0.5f);
            U16Data[1] = static_cast<UInt16>(ClampedG * 65535.0f + 0.5f);
            U16Data[2] = static_cast<UInt16>(ClampedB * 65535.0f + 0.5f);
            break;
        }
        case EPixelFormat::RGBA16_UNorm:
        {
            UInt16* U16Data = reinterpret_cast<UInt16*>(Data);
            U16Data[0] = static_cast<UInt16>(ClampedR * 65535.0f + 0.5f);
            U16Data[1] = static_cast<UInt16>(ClampedG * 65535.0f + 0.5f);
            U16Data[2] = static_cast<UInt16>(ClampedB * 65535.0f + 0.5f);
            U16Data[3] = static_cast<UInt16>(ClampedA * 65535.0f + 0.5f);
            break;
        }

        // 16-bit Float formats
        case EPixelFormat::R16_Float:
        {
            UInt16* HalfBits = reinterpret_cast<UInt16*>(Data);
            const FHalf HalfValue(ClampedR);
            HalfBits[0] = HalfValue.ToBits();
            break;
        }
        case EPixelFormat::RG16_Float:
        {
            UInt16* HalfBits = reinterpret_cast<UInt16*>(Data);
            const FHalf HalfR(ClampedR);
            const FHalf HalfG(ClampedG);
            HalfBits[0] = HalfR.ToBits();
            HalfBits[1] = HalfG.ToBits();
            break;
        }
        case EPixelFormat::RGB16_Float:
        {
            UInt16* HalfBits = reinterpret_cast<UInt16*>(Data);
            const FHalf HalfR(ClampedR);
            const FHalf HalfG(ClampedG);
            const FHalf HalfB(ClampedB);
            HalfBits[0] = HalfR.ToBits();
            HalfBits[1] = HalfG.ToBits();
            HalfBits[2] = HalfB.ToBits();
            break;
        }
        case EPixelFormat::RGBA16_Float:
        {
            UInt16* HalfBits = reinterpret_cast<UInt16*>(Data);
            const FHalf HalfR(ClampedR);
            const FHalf HalfG(ClampedG);
            const FHalf HalfB(ClampedB);
            const FHalf HalfA(ClampedA);
            HalfBits[0] = HalfR.ToBits();
            HalfBits[1] = HalfG.ToBits();
            HalfBits[2] = HalfB.ToBits();
            HalfBits[3] = HalfA.ToBits();
            break;
        }

        // 32-bit Float formats
        case EPixelFormat::R32_Float:
        {
            Float* FloatData = reinterpret_cast<Float*>(Data);
            FloatData[0] = ClampedR;
            break;
        }
        case EPixelFormat::RG32_Float:
        {
            Float* FloatData = reinterpret_cast<Float*>(Data);
            FloatData[0] = ClampedR;
            FloatData[1] = ClampedG;
            break;
        }
        case EPixelFormat::RGB32_Float:
        {
            Float* FloatData = reinterpret_cast<Float*>(Data);
            FloatData[0] = ClampedR;
            FloatData[1] = ClampedG;
            FloatData[2] = ClampedB;
            break;
        }
        case EPixelFormat::RGBA32_Float:
        {
            Float* FloatData = reinterpret_cast<Float*>(Data);
            FloatData[0] = ClampedR;
            FloatData[1] = ClampedG;
            FloatData[2] = ClampedB;
            FloatData[3] = ClampedA;
            break;
        }

        // Special formats
        case EPixelFormat::RGBE8_HDR:
        {
            // RGBE format: 3x8-bit RGB + 8-bit exponent
            // Encode HDR values using shared exponent representation
            UInt8* U8Data = reinterpret_cast<UInt8*>(Data);
            const Float MaxChannel = Math::Max(Math::Max(ClampedR, ClampedG), ClampedB);
            if (MaxChannel < 1e-32f)
            {
                U8Data[0] = 0;
                U8Data[1] = 0;
                U8Data[2] = 0;
                U8Data[3] = 0;
            }
            else
            {
                Int32 Exponent;
                const Float Mantissa = std::frexp(MaxChannel, &Exponent);
                const Float NormalizedMax = Mantissa * 256.0f;
                U8Data[0] = static_cast<UInt8>(Math::Clamp(ClampedR / MaxChannel * NormalizedMax, 0.0f, 255.0f));
                U8Data[1] = static_cast<UInt8>(Math::Clamp(ClampedG / MaxChannel * NormalizedMax, 0.0f, 255.0f));
                U8Data[2] = static_cast<UInt8>(Math::Clamp(ClampedB / MaxChannel * NormalizedMax, 0.0f, 255.0f));
                U8Data[3] = static_cast<UInt8>(Exponent + 128);
            }
            break;
        }

        case EPixelFormat::Invalid:
        default:
            // Unsupported format - do nothing
            break;
        }
    }
}