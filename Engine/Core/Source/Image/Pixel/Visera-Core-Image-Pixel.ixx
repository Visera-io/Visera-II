module;
#include <Visera-Core.hpp>
export module Visera.Core.Image.Pixel;
#define VISERA_MODULE_NAME "Core.Image"
export import Visera.Core.Image.Common;

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
        [[nodiscard]] FByte* GetData() const { return Data; }

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

    private:
        FByte*       Data;        // Pointer to pixel data
        EPixelFormat PixelFormat; // Format of the pixel
        UInt8        BytesPerPixel; // Bytes per pixel (cached for performance)

    public:
        /**
         * Constructs a pixel view from raw data pointer and format.
         * @param I_Data Pointer to pixel data
         * @param I_PixelFormat Format of the pixel
         * @param I_BytesPerPixel Bytes per pixel (cached for performance)
         */
        explicit FPixel(FByte* I_Data, EPixelFormat I_PixelFormat, UInt8 I_BytesPerPixel)
            : Data{I_Data}
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
}