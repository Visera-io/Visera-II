module;
#include <Visera-Engine.hpp>
export module Visera.Engine.AssetHub;
#define VISERA_MODULE_NAME "Engine.AssetHub"
export import Visera.Engine.AssetHub.Sound;
export import Visera.Engine.AssetHub.Shader;
export import Visera.Engine.AssetHub.Texture;
       import Visera.Engine.AssetHub.Asset;
       import Visera.Runtime.Platform;
       import Visera.Core.Log;
       import Visera.Core.OS.FileSystem;

namespace Visera
{
    export using EAssetType = IAsset::EType;

    export enum class EAssetSource : UInt8
    { App, Studio, Engine, Any, };

    export class VISERA_ENGINE_API FAssetHub : public IGlobalSingleton<FAssetHub>
    {
    public:
        [[nodiscard]] inline TSharedPtr<FTexture>
        LoadTexture(const FPath& I_File, EAssetSource I_Source = EAssetSource::Any);
        [[nodiscard]] inline TSharedPtr<FShader>
        LoadShader(const FPath& I_File, const FString& I_EntryPoint, EAssetSource I_Source = EAssetSource::Any);
        [[nodiscard]] inline TSharedPtr<FSound>
        LoadSound(const FPath& I_File, EAssetSource I_Source = EAssetSource::Any);

        [[nodiscard]] inline const FPath&
        GetAssetDirectory(EAssetSource I_Source) const { VISERA_ASSERT(I_Source != EAssetSource::Any); return Roots.at(I_Source).GetRoot(); }

    public:
        void Bootstrap() override;
        void Terminate() override;
        FAssetHub() : IGlobalSingleton("AssetHub") {};
        ~FAssetHub() override= default;

    private:
        TMap<EAssetSource, FFileSystem> Roots;
    };

    export inline VISERA_ENGINE_API TUniquePtr<FAssetHub>
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

    TSharedPtr<FTexture> FAssetHub::
    LoadTexture(const FPath& I_File, EAssetSource I_Source /* = EAssetSource::Any */)
    {
        VISERA_ASSERT(IsBootstrapped());

        TSharedPtr<FTexture> Texture{};

        if (I_Source != EAssetSource::Any)
        {
            const auto& Root = Roots[I_Source];
            FPath Path = Root.GetRoot() / PATH("Texture") / I_File;

            if (FFileSystem::Exists(Path) && !FFileSystem::IsDirectory(Path))
            {
                Texture = MakeShared<FTexture>(FName{Path.GetUTF8Path()}, Path);
            }
        }
        else
        {
            for (const auto& [_, Root] : Roots)
            {
                FPath Path = Root.GetRoot() / PATH("Texture") / I_File;

                if (FFileSystem::Exists(Path) && !FFileSystem::IsDirectory(Path))
                {
                    Texture = MakeShared<FTexture>(FName{Path.GetUTF8Path()}, Path);
                    break;
                }
            }
        }

        if (!Texture)
        {
            LOG_ERROR("Failed to load the texture \"{}\"!", I_File);
            return {};
        }

        LOG_DEBUG("Loaded the texture \"{}\".",
                  Texture->GetPath());

        return Texture;
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
