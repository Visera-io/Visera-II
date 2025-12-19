module;
#include <Visera-Assets.hpp>
export module Visera.Assets.Image.Common;
#define VISERA_MODULE_NAME "Assets.Image"

export namespace Visera
{
    /**
     * Enumerates the types of image formats this class can handle.
     */
    enum class EImageFormat : Int8
    {
        /** Invalid or unrecognized format. */
        Invalid = -1,
        /** Portable Network Graphics. */
        PNG = 0,
        /** Joint Photographic Experts Group. */
        //JPEG,
        /** Single channel JPEG. */
        //GrayscaleJPEG,
        /** Windows Bitmap. */
        //BMP,
        /** Windows Icon resource. */
        //ICO,
        /** OpenEXR (HDR) image file format. */
        //EXR,
        /** Mac icon. */
        //ICNS,
        /** Hdr file from radiance using RGBE */
        //HDR,
    };

    /**
     * Enumerates the types of Image formats this class can handle.
     */
    enum class EColorFormat : Int8
    {
        Invalid   = -1,
        // Red, Green, Blue
        RGB,
        // Red, Green, Blue and Alpha ; requires RB swap from FColor
        RGBA,
        // Blue, Green, Red
        BGR,
        // Blue, Green, Red and Alpha ; is ERawImageFormat::BGRA8 and FColor
        BGRA,
        // Gray scale
        Gray,
        // Red, Green, Blue and Alpha using IEEE Floating-Point Arithmetic (see IEEE754). The format is always binary.
        RGBAFloat,
        // Blue, Green, Red and Exponent (Similar to the RGBE format from radiance but with the blue and red channel inversed)
        BGRExp,
        // Gray scale using IEEE Floating-Point Arithmetic (see IEEE754). The format is always binary.
        GrayFloat,
    };
}
