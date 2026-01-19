module;
#include <Visera-Core.hpp>
#include <stb_image_resize2.h>
export module Visera.Core.Image.Common;
#define VISERA_MODULE_NAME "Core.Image"

export namespace Visera
{
    /**
     * Enumerates color space encoding.
     */
    enum class EColorSpace : UInt8
    {
        /** Linear color space (no gamma encoding) */
        Linear,
        /** sRGB color space (gamma encoded) */
        sRGB,
    };

    /**
     * Enumerates pixel storage formats.
     * Each format fully defines the storage layout, bit depth, and data type.
     */
    enum class EPixelFormat : UInt8
    {
        // 8-bit unsigned normalized formats
        R8_UNorm,          /** Single channel, 8-bit unsigned normalized */
        RG8_UNorm,         /** Two channels, 8-bit unsigned normalized */
        RGB8_UNorm,        /** Three channels, 8-bit unsigned normalized */
        RGBA8_UNorm,       /** Four channels, 8-bit unsigned normalized */
        BGRA8_UNorm,       /** Four channels (BGR order), 8-bit unsigned normalized */

        // 16-bit unsigned normalized formats
        R16_UNorm,         /** Single channel, 16-bit unsigned normalized */
        RG16_UNorm,        /** Two channels, 16-bit unsigned normalized */
        RGB16_UNorm,       /** Three channels, 16-bit unsigned normalized */
        RGBA16_UNorm,      /** Four channels, 16-bit unsigned normalized */

        // 16-bit floating point formats
        R16_Float,          /** Single channel, 16-bit float */
        RG16_Float,         /** Two channels, 16-bit float */
        RGB16_Float,        /** Three channels, 16-bit float */
        RGBA16_Float,       /** Four channels, 16-bit float */

        // 32-bit floating point formats
        R32_Float,          /** Single channel, 32-bit float */
        RG32_Float,         /** Two channels, 32-bit float */
        RGB32_Float,        /** Three channels, 32-bit float */
        RGBA32_Float,       /** Four channels, 32-bit float */

        // Special packed formats
        RGBE8_HDR,          /** RGBExp format: 3x8-bit RGB + 8-bit shared exponent (HDR packed) */
        
        /** Invalid or unrecognized format */
        Invalid,
    };
}