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
     * Interface for image format loaders.
     * Each image format (PNG, JPEG, etc.) should implement this interface.
     * Internal use only, not exposed to users.
     */
    class VISERA_ASSETHUB_API IImageWrapper
    {
    public:
        /**
         * Imports image from a file path and creates an FImage.
         * @param I_Path The path to the image file
         * @return A shared pointer to the loaded FImage, or nullptr on failure
         */
        [[nodiscard]] virtual TSharedPtr<FImage>
        Import(const FPath& I_Path) = 0;

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
        const FPath Extension = I_Path.GetExtension();
        const FString ExtStr = Extension.GetUTF8Path();
        
        if (ExtStr == ".png" || ExtStr == ".PNG")
        { return EImageFormat::PNG; }

        if (ExtStr == ".exr" || ExtStr == ".EXR")
        { return EImageFormat::EXR; }

        // OpenJPH supports JPEG2000 / HTJ2K codestreams (J2K/J2C/JPH).
        if (ExtStr == ".jp2" || ExtStr == ".JP2" ||
            ExtStr == ".j2k" || ExtStr == ".J2K" ||
            ExtStr == ".j2c" || ExtStr == ".J2C" ||
            ExtStr == ".jph" || ExtStr == ".JPH")
        { return EImageFormat::JPEG2000; }

        return EImageFormat::Invalid;
    }
}