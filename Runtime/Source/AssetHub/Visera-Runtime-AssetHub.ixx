module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.AssetHub;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
import Visera.Runtime.AssetHub.Asset;
import Visera.Runtime.AssetHub.Image;
import Visera.Runtime.AssetHub.Sound;
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
        [[nodiscard]] inline TSharedPtr<FSound>
        LoadSound(const FPath& I_File, EAssetSource I_Source = EAssetSource::Any);

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
            FPath Path = Root.GetRoot() / PATH("Image") / I_File;

            if (FFileSystem::Exists(Path) && !FFileSystem::IsDirectory(Path))
            {
                Image = MakeShared<FImage>(FName{Path.GetUTF8Path()}, Path);
            }
        }
        else
        {
            for (const auto& [_, Root] : Roots)
            {
                FPath Path = Root.GetRoot() / PATH("Image") / I_File;

                if (FFileSystem::Exists(Path) && !FFileSystem::IsDirectory(Path))
                {
                    Image = MakeShared<FImage>(FName{Path.GetUTF8Path()}, Path);
                    break;
                }
            }
        }

        if (!Image)
        { LOG_ERROR("Failed to load the image \"{}\"", I_File); }

        LOG_DEBUG("Loaded the image \"{}\" (extend:[{},{}], sRGB:{}).",
                  Image->GetPath(), Image->GetWidth(), Image->GetHeight(), Image->IsSRGB());

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
            FPath Path = Root.GetRoot() / PATH("Shader") / I_File;

            if (FFileSystem::Exists(Path) && !FFileSystem::IsDirectory(Path))
            {
                Shader = MakeShared<FShader>(FName{Path.GetUTF8Path()}, Path, I_EntryPoint);
            }
        }
        else
        {
            for (const auto& [_, Root] : Roots)
            {
                FPath Path = Root.GetRoot() / PATH("Shader") / I_File;

                if (FFileSystem::Exists(Path) && !FFileSystem::IsDirectory(Path))
                {
                    Shader = MakeShared<FShader>(FName{Path.GetUTF8Path()}, Path, I_EntryPoint);
                    break;
                }
            }
        }

        if (!Shader)
        { LOG_ERROR("Failed to load the shader \"{}\"", I_File); }

        LOG_DEBUG("Loaded the shader \"{}\" (stage:{}, entry_point:{}, size:{}).",
                  Shader->GetPath(), Shader->GetStage(), Shader->GetEntryPoint(), Shader->GetSize());

        return Shader;
    }

    TSharedPtr<FSound> FAssetHub::
    LoadSound(const FPath& I_File, EAssetSource I_Source /* = EAssetSource::Any */)
    {
        VISERA_ASSERT(IsBootstrapped());

        TSharedPtr<FSound> Sound{};

        if (I_Source != EAssetSource::Any)
        {
            const auto& Root = Roots[I_Source];
            FPath Path = Root.GetRoot() / PATH("Sound") /
#if defined(VISERA_ON_WINDOWS_SYSTEM)
                PATH("Windows")
#elif defined(VISERA_ON_APPLE_SYSTEM)
                PATH("Mac")
#else
                VISERA_UNIMPLEMENTED_API;
#endif
                / I_File;

            if (FFileSystem::Exists(Path) && !FFileSystem::IsDirectory(Path))
            {
                Sound = MakeShared<FSound>(FName{Path.GetUTF8Path()}, Path);
            }
        }
        else
        {
            for (const auto& [_, Root] : Roots)
            {
                FPath Path = Root.GetRoot() / PATH("Sound") /
#if defined(VISERA_ON_WINDOWS_SYSTEM)
                PATH("Windows")
#elif defined(VISERA_ON_APPLE_SYSTEM)
                PATH("Mac")
#else
                VISERA_UNIMPLEMENTED_API;
#endif
                / I_File;

                if (FFileSystem::Exists(Path) && !FFileSystem::IsDirectory(Path))
                {
                    Sound = MakeShared<FSound>(FName{Path.GetUTF8Path()}, Path);
                    break;
                }
            }
        }

        if (!Sound || Sound->IsInvalid())
        {
            LOG_ERROR("Failed to load the sound \"{}\"!", I_File);
            return {};
        }

        LOG_DEBUG("Loaded the sound \"{}\" (id:{}).",
                  Sound->GetPath(), Sound->GetID());
        return Sound;
    }
}
