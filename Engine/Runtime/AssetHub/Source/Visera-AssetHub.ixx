module;
#include <Visera-AssetHub.hpp>
export module Visera.AssetHub;
#define VISERA_MODULE_NAME "AssetHub"
export import Visera.Core.Image;
export import Visera.Core.Types.Path;
       import Visera.Global;
       import Visera.AssetHub.Image.Wrapper;
       import Visera.AssetHub.Image.PNG;
       import Visera.Global.Log;

export namespace Visera
{
    class VISERA_ASSETHUB_API FAssetHub : public IGlobalService
    {
    public:
        [[nodiscard]] TSharedPtr<FImage>
        LoadImage(const FPath& I_Path);

    private:

    public:
        FAssetHub() : IGlobalService(EName::AssetHub)
        {
            Dependencies =
            {
                EName::Platform,
            };

            if (!OnBootstrap.TryBind([this]
            {
                return True;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        }
    };

    /**
     * Loads an image from a file path and creates an FImage.
     * Automatically detects the image format and uses the appropriate loader.
     * @param I_Path The path to the image file
     * @return A shared pointer to the loaded FImage, or nullptr on failure
     */
    TSharedPtr<FImage> FAssetHub::
    LoadImage(const FPath& I_Path)
    {
        // Detect image format from extension
        const EImageFormat Format = DetectImageFormat(I_Path);
        
        if (Format == EImageFormat::Invalid)
        {
            LOG_ERROR("Failed to detect image format for: {}", I_Path);
            return nullptr;
        }

        // Create appropriate wrapper and load image
        TUniquePtr<IImageWrapper> Wrapper;
        switch (Format)
        {
        case EImageFormat::PNG:
            Wrapper = MakeUnique<FPNGImageWrapper>();
            break;
        
        default:
            LOG_ERROR("Unsupported image format for: {}", I_Path);
            return nullptr;
        }

        // Import image directly (returns TSharedPtr)
        return Wrapper->Import(I_Path);
    }
}