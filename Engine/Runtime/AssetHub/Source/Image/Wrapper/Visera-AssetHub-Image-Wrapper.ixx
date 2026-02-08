module;
#include <Visera-AssetHub.hpp>
export module Visera.AssetHub.Image.Wrapper;
#define VISERA_MODULE_NAME "AssetHub.Image"
export import Visera.AssetHub.Image.Common;
export import Visera.Core.Image;
export import Visera.Core.Types.Path;
export import Visera.Core.Types.Pointer;
       import Visera.Core.Types.String;
       import Visera.Global.Log;

export namespace Visera
{
    /**
     * Interface for image format loaders and exporters.
     * Each image format (PNG, JPEG, etc.) should implement this interface.
     * Internal use only, not exposed to users.
     */
    class VISERA_ASSETHUB_API IImageWrapper
    {
    public:
        /**
         * Imports image from a file path.
         * @param I_Path The path to the image file
         * @return Loaded FImage, or empty (0x0) FImage on failure
         */
        [[nodiscard]] virtual FImage
        Import(const FPath& I_Path) = 0;

        /**
         * Exports an FImage to a file path. Takes pure data (const FImage&) to avoid multi-thread write issues.
         * @param I_Image The image data to export
         * @param I_Path The path to save the image file
         * @return True if successful, false otherwise
         */
        [[nodiscard]] virtual Bool
        Export(const FImage& I_Image, const FPath& I_Path) = 0;

    public:
        virtual ~IImageWrapper() = default;
    };

    /**
     * Detects the image format from file extension only.
     * @param I_Path The file path
     * @return The detected image format, or EImageFormat::Invalid if unknown
     */
    [[nodiscard]] inline EImageFormat
    DetectImageFormat(const FPath& I_Path)
    {
        if (auto R = I_Path.GetExtension(); R.HasValue())
        {
            const FStringView Extension = *R;

            if (Extension == ".png" || Extension == ".PNG")
            { return EImageFormat::PNG; }

            if (Extension == ".exr" || Extension == ".EXR")
            { return EImageFormat::EXR; }

            // OpenJPH supports JPEG2000 / HTJ2K codestreams (J2K/J2C/JPH).
            if (Extension == ".jp2" || Extension == ".JP2" ||
                Extension == ".j2k" || Extension == ".J2K" ||
                Extension == ".j2c" || Extension == ".J2C" ||
                Extension == ".jph" || Extension == ".JPH")
            { return EImageFormat::JPEG2000; }
        }
        return EImageFormat::Invalid;
    }
}