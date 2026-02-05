module;
#include <Visera-AssetHub.hpp>
export module Visera.AssetHub;
#define VISERA_MODULE_NAME "AssetHub"
export import Visera.Core.Types.Path;
       import Visera.AssetHub.Image;
       import Visera.Platform.OS;
       import Visera.Global;

export namespace Visera
{
    class VISERA_ASSETHUB_API FAssetHub : public IGlobalService
    {
    public:
        [[nodiscard]] TSharedPtr<FImage>
        LoadImage(const FPath& I_Path);

    private:
        FPlatform* Platform {nullptr};

    public:
        FAssetHub() : IGlobalService(EName::AssetHub)
        {
            Dependencies =
            {
                EName::Platform,
                EName::Tasks,
            };

            if (!OnBootstrap.TryBind([this]
            {
                Platform = IGlobalService::Get<FPlatform>(EName::Platform);
                return Platform;
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
        const auto& Path = I_Path; //Platform->GetResourceDirectory() / I_Path;
        // Detect image format from extension
        const EImageFormat Format = DetectImageFormat(Path);
        
        if (Format == EImageFormat::Invalid)
        {
            LOG_ERROR("Failed to detect image format for: {}", Path);
            return nullptr;
        }

        // Create appropriate wrapper and load image
        TUniquePtr<IImageWrapper> Wrapper;
        switch (Format)
        {
        case EImageFormat::PNG:
            Wrapper = MakeUnique<FPNGImageWrapper>();
            break;

        case EImageFormat::EXR:
            Wrapper = MakeUnique<FEXRImageWrapper>();
            break;
        
        default:
            LOG_ERROR("Unsupported image format for: {}", Path);
            return nullptr;
        }

        // Import image directly (returns TSharedPtr)
        return Wrapper->Import(Path);
    }
}