module;
#include <Visera-Core.hpp>
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

VISERA_MAKE_FORMATTER(Visera::EColorSpace,
    const char* ColorSpaceName = "Unknown";
    switch (I_Formatee)
    {
        case Visera::EColorSpace::Linear: ColorSpaceName = "Linear"; break;
        case Visera::EColorSpace::sRGB:   ColorSpaceName = "sRGB";   break;
        default: break;
    },
    "{}", ColorSpaceName
);

VISERA_MAKE_FORMATTER(Visera::EPixelFormat,
    const char* FormatName = "Invalid";
    switch (I_Formatee)
    {
        case Visera::EPixelFormat::R8_UNorm:        FormatName = "R8_UNorm";        break;
        case Visera::EPixelFormat::RG8_UNorm:       FormatName = "RG8_UNorm";       break;
        case Visera::EPixelFormat::RGB8_UNorm:      FormatName = "RGB8_UNorm";      break;
        case Visera::EPixelFormat::RGBA8_UNorm:     FormatName = "RGBA8_UNorm";     break;
        case Visera::EPixelFormat::BGRA8_UNorm:     FormatName = "BGRA8_UNorm";     break;
        case Visera::EPixelFormat::R16_UNorm:       FormatName = "R16_UNorm";       break;
        case Visera::EPixelFormat::RG16_UNorm:      FormatName = "RG16_UNorm";      break;
        case Visera::EPixelFormat::RGB16_UNorm:     FormatName = "RGB16_UNorm";     break;
        case Visera::EPixelFormat::RGBA16_UNorm:    FormatName = "RGBA16_UNorm";    break;
        case Visera::EPixelFormat::R16_Float:       FormatName = "R16_Float";       break;
        case Visera::EPixelFormat::RG16_Float:      FormatName = "RG16_Float";      break;
        case Visera::EPixelFormat::RGB16_Float:     FormatName = "RGB16_Float";     break;
        case Visera::EPixelFormat::RGBA16_Float:    FormatName = "RGBA16_Float";    break;
        case Visera::EPixelFormat::R32_Float:       FormatName = "R32_Float";       break;
        case Visera::EPixelFormat::RG32_Float:      FormatName = "RG32_Float";      break;
        case Visera::EPixelFormat::RGB32_Float:     FormatName = "RGB32_Float";     break;
        case Visera::EPixelFormat::RGBA32_Float:    FormatName = "RGBA32_Float";    break;
        case Visera::EPixelFormat::RGBE8_HDR:       FormatName = "RGBE8_HDR";       break;
        case Visera::EPixelFormat::Invalid:         FormatName = "Invalid";         break;
        default: break;
    },
    "{}", FormatName
);