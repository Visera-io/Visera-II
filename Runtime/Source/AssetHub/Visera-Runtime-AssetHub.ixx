module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.AssetHub;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
import Visera.Runtime.AssetHub.Asset;
import Visera.Runtime.AssetHub.Image;
import Visera.Runtime.AssetHub.Shader;
import Visera.Runtime.Platform;
import Visera.Core.Log;
import Visera.Core.OS.FileSystem;

namespace Visera
{
    export using EAssetType = IAsset::EType;

    export enum class EAssetSource : UInt8
    { App, Studio, Engine, Any, };

    class VISERA_RUNTIME_API FAssetHub : public IGlobalSingleton
    {
    public:
        [[nodiscard]] inline TSharedPtr<FImage>
        LoadImage(const FPath& I_File, EAssetSource I_Source = EAssetSource::Any);
        [[nodiscard]] inline TSharedPtr<FShader>
        LoadShader(const FPath& I_File, const FString& I_EntryPoint, EAssetSource I_Source = EAssetSource::Any);

    public:
        void Bootstrap() override;
        void Terminate() override;
        FAssetHub() : IGlobalSingleton("AssetHub") {};
        ~FAssetHub() override= default;

    private:
        TMap<EAssetSource, FFileSystem> Roots;
    };

    export inline VISERA_RUNTIME_API auto
    GAssetHub = MakeUnique<FAssetHub>();

    void FAssetHub::
    Bootstrap()
    {
        LOG_TRACE("Bootstrapping AssetHub.");

        Roots[EAssetSource::Engine] = FFileSystem{FPath(VISERA_ENGINE_DIR) / PATH("Assets") };
        Roots[EAssetSource::Studio] = FFileSystem{FPath(VISERA_STUDIO_DIR) / PATH("Assets") };
        Roots[EAssetSource::App]    = FFileSystem{FPath(VISERA_APP_DIR)    / PATH("Assets") };
        
        Status = EStatus::Bootstrapped;
    }

    void FAssetHub::
    Terminate()
    {
        LOG_TRACE("Terminating AssetHub.");

        Status = EStatus::Terminated;
    }

    TSharedPtr<FImage> FAssetHub::
    LoadImage(const FPath& I_File, EAssetSource I_Source /* = EAssetSource::Any */)
    {
        VISERA_ASSERT(IsBootstrapped());

        TSharedPtr<FImage> Image{};

        if (I_Source != EAssetSource::Any)
        {
            const auto& Root = Roots[I_Source];
            FPath Path = Root.GetRoot() / PATH("Images") / I_File;
            LOG_TRACE("Searching the image in \"{}\".", Path);

            if (FFileSystem::Exists(Path) && !FFileSystem::IsDirectory(Path))
            {
                Image = MakeShared<FImage>(FName{Path.GetUTF8Path()}, Path);
            }
        }
        else
        {
            for (const auto& [_, Root] : Roots)
            {
                FPath Path = Root.GetRoot() / PATH("Images") / I_File;
                LOG_TRACE("Searching the image in \"{}\".", Path);

                if (FFileSystem::Exists(Path) && !FFileSystem::IsDirectory(Path))
                {
                    Image = MakeShared<FImage>(FName{Path.GetUTF8Path()}, Path);
                    break;
                }
            }
        }

        if (!Image)
        { LOG_ERROR("Failed to load the image \"{}\"", I_File); }

        return Image;
    }

    TSharedPtr<FShader> FAssetHub::
    LoadShader(const FPath& I_File, const FString& I_EntryPoint, EAssetSource I_Source /* = EAssetSource::Any */)
    {
        VISERA_ASSERT(IsBootstrapped());

        TSharedPtr<FShader> Shader{};

        if (I_Source != EAssetSource::Any)
        {
            const auto& Root = Roots[I_Source];
            FPath Path = Root.GetRoot() / PATH("Shaders") / I_File;
            LOG_TRACE("Searching the shader in \"{}\".", Path);

            if (FFileSystem::Exists(Path) && !FFileSystem::IsDirectory(Path))
            {
                Shader = MakeShared<FShader>(FName{Path.GetUTF8Path()}, Path, I_EntryPoint);
            }
        }
        else
        {
            for (const auto& [_, Root] : Roots)
            {
                FPath Path = Root.GetRoot() / PATH("Shaders") / I_File;
                LOG_TRACE("Searching the shader in \"{}\".", Path);

                if (FFileSystem::Exists(Path) && !FFileSystem::IsDirectory(Path))
                {
                    Shader = MakeShared<FShader>(FName{Path.GetUTF8Path()}, Path, I_EntryPoint);
                    break;
                }
            }
        }

        if (!Shader)
        { LOG_ERROR("Failed to load the image \"{}\"", I_File); }

        return Shader;
    }
}
