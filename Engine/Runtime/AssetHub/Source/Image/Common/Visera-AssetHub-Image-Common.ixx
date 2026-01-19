module;
#include <Visera-AssetHub.hpp>
export module Visera.AssetHub.Image.Common;
#define VISERA_MODULE_NAME "AssetHub.Image"
export import Visera.Core.Image;

export namespace Visera
{
    /**
     * Enumerates the types of image file formats this class can handle.
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
}
