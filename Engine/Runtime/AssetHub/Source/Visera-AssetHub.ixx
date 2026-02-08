module;
#include <Visera-AssetHub.hpp>
export module Visera.AssetHub;
#define VISERA_MODULE_NAME "AssetHub"
export import Visera.Core.Types.Path;
export import Visera.AssetHub.Image;
export import Visera.AssetHub.Shader;
export import Visera.AssetHub.Font;
       import Visera.AssetHub.Asset;
       import Visera.Core.Types.Pointer;
       import Visera.Core.Meta.Cast;
       import Visera.Core.Types.Map;
       import Visera.Core.Types.Array;
       import Visera.Core.Types.String;
       import Visera.Core.OS.Thread.Sync;
       import Visera.Core.OS.FileSystem;
       import Visera.Core.Image;
       import Visera.Core.Font;
       import Visera.Global;

export namespace Visera
{
    class VISERA_ASSETHUB_API FAssetHub : public IGlobalService
    {
    public:
        /** Load image; returns read-only FImageAsset (IAsset). Use Save(const FImage&, path) to write. */
        [[nodiscard]] TSharedPtr<FImageAsset>
        LoadImage(const FPath& I_Path);
        /** Save image data to file (pure data FImage; avoids multi-thread write via asset handle). */
        [[nodiscard]] Bool
        SaveImage(const FImage& I_Image, const FPath& I_Path);
        /** Load .vshader from file. Returns read-only asset (IAsset). */
        [[nodiscard]] TSharedPtr<FShaderAsset>
        LoadShader(const FPath& I_Path);
        /** Save shader data to .vshader file (pure data FShader; use FShader::Write* for custom serialization). */
        [[nodiscard]] Bool
        SaveShader(const FShader& I_Shader, const FPath& I_Path);
        /** Load font face from file. Optional I_PixelSize: when > 0, size is set at load time (cached per path+face+size). Returns read-only FFontAsset (IAsset). */
        [[nodiscard]] TSharedPtr<FFontAsset>
        LoadFont(const FPath& I_Path, Int32 I_FaceIndex = 0, UInt32 I_PixelSize = 0);

        /** Get cached image by name (same name as used when loaded by path, e.g. FName(I_Path.GetString())). Returns nullptr if not found or cast fails. */
        [[nodiscard]] TSharedPtr<FImageAsset>
        LoadImageFromCache(const FName& I_Name);
        /** Get cached shader by name. Returns nullptr if not found or cast fails. */
        [[nodiscard]] TSharedPtr<FShaderAsset>
        LoadShaderFromCache(const FName& I_Name);
        /** Get cached font by name (font cache name is path_faceIndex_pixelSize). Returns nullptr if not found or cast fails. */
        [[nodiscard]] TSharedPtr<FFontAsset>
        LoadFontFromCache(const FName& I_Name);

    private:
        template<typename T>
        class TCache
        {
        public:
            [[nodiscard]] TWeakPtr<T>
            Find(const FName& I_Key) const
            {
                FScopeReadLock _{&RWLock};
                if (auto It = Entries.Find(I_Key); It != Entries.end())
                { return It->second; }
                return {};
            }

            [[nodiscard]] Bool
            Store(const FName& I_Key, TSharedRef<T> I_Value)
            {
                FScopeWriteLock _{&RWLock};
                auto& Entry = Entries[I_Key];
                if (Entry.IsExpired())
                {
                    Entry = I_Value;
                    return True;
                }
                return False;
            }

        private:
            mutable FRWLock          RWLock;
            TMap<FName, TWeakPtr<T>> Entries;
        };
        /** Unified cache by IAsset; Load* methods cast to concrete type. */
        TCache<IAsset> AssetCache;

    public:
        FAssetHub() : IGlobalService(EName::AssetHub)
        {
            Dependencies =
            {
                EName::Tasks,
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

    TSharedPtr<FImageAsset> FAssetHub::
    LoadImage(const FPath& I_Path)
    {
        const FName PathName{I_Path.GetString()};
        if (auto W = AssetCache.Find(PathName); !W.IsExpired())
        {
            LOG_TRACE("Get {} from cache.", I_Path);
            return Cast<FImageAsset>(W.Lock());
        }

        const EImageFormat Format = DetectImageFormat(I_Path);
        if (Format == EImageFormat::Invalid)
        {
            LOG_ERROR("Failed to detect image format for: {}", I_Path);
            return nullptr;
        }

        TUniquePtr<IImageWrapper> Wrapper;
        switch (Format)
        {
        case EImageFormat::PNG:  Wrapper = MakeUnique<FPNGImageWrapper>(); break;
        case EImageFormat::EXR:  Wrapper = MakeUnique<FEXRImageWrapper>(); break;
        default:
            LOG_ERROR("Unsupported image format for: {}", I_Path);
            return nullptr;
        }

        FImage NewImage = Wrapper->Import(I_Path);
        if (NewImage.GetWidth() == 0) return nullptr;

        auto NewAsset = MakeShared<FImageAsset>(std::move(NewImage));
        if (!AssetCache.Store(PathName, Cast<IAsset>(NewAsset)))
        { LOG_WARN("Failed to store {} to asset cache!", I_Path); }
        return NewAsset;
    }

    Bool FAssetHub::
    SaveImage(const FImage& I_Image, const FPath& I_Path)
    {
        if (I_Image.GetWidth() == 0 || I_Image.GetHeight() == 0)
        {
            LOG_ERROR("Image has invalid dimensions ({}x{}) for saving: {}",
                     I_Image.GetWidth(), I_Image.GetHeight(), I_Path);
            return False;
        }
        if (I_Image.GetPixelFormat() == EPixelFormat::Invalid)
        {
            LOG_ERROR("Image has invalid pixel format for saving: {}", I_Path);
            return False;
        }

        const Bool IsFloatFormat = I_Image.IsFloatFormat();
        const EImageFormat TargetFormat = IsFloatFormat ? EImageFormat::EXR : EImageFormat::PNG;

        TUniquePtr<IImageWrapper> Wrapper;
        switch (TargetFormat)
        {
        case EImageFormat::PNG:  Wrapper = MakeUnique<FPNGImageWrapper>(); break;
        case EImageFormat::EXR:  Wrapper = MakeUnique<FEXRImageWrapper>(); break;
        default:
            LOG_ERROR("Unsupported pixel format for saving: {}", I_Path);
            return False;
        }
        return Wrapper->Export(I_Image, I_Path);
    }

    TSharedPtr<FShaderAsset> FAssetHub::
    LoadShader(const FPath& I_Path)
    {
        const FName PathName{I_Path.GetString()};
        if (auto W = AssetCache.Find(PathName); !W.IsExpired())
        {
            LOG_TRACE("Get {} from shader cache.", I_Path);
            return Cast<FShaderAsset>(W.Lock());
        }

        TArray<FByte> SPIRVChunk, ReflectionChunk;
        UInt32 Version = 0;
        if (!ReadShaderChunks(I_Path, Version, SPIRVChunk, ReflectionChunk) || SPIRVChunk.IsEmpty())
        { return nullptr; }
        FShader::FLayout Refl;
        if (ReflectionChunk.IsEmpty() || !DeserializeShaderReflection(Version, FStringView(reinterpret_cast<const char*>(ReflectionChunk.Data()), ReflectionChunk.GetSize()), Refl))
        { return nullptr; }
        if (Refl.EntryPoints.IsEmpty())
        { return nullptr; }
        auto NewShader = MakeShared<FShaderAsset>(FShader{std::move(SPIRVChunk), std::move(Refl)});
        if (!AssetCache.Store(PathName, Cast<IAsset>(NewShader)))
        { LOG_WARN("Failed to store {} to asset cache!", I_Path); }
        return NewShader;
    }

    Bool FAssetHub::
    SaveShader(const FShader& I_Shader, const FPath& I_Path)
    {
        return WriteShaderToFile(I_Shader, I_Path);
    }

    TSharedPtr<FFontAsset> FAssetHub::
    LoadFont(const FPath& I_Path, Int32 I_FaceIndex, UInt32 I_PixelSize)
    {
        const FString CacheKeyStr = FString::Format("{}_{}_{}", I_Path.GetString(), I_FaceIndex, I_PixelSize);
        const FName CacheKey{CacheKeyStr};

        if (auto W = AssetCache.Find(CacheKey); !W.IsExpired())
        {
            LOG_TRACE("Get font {} (face {}, size {}) from cache.", I_Path, I_FaceIndex, I_PixelSize);
            return Cast<FFontAsset>(W.Lock());
        }

        auto File = FFileSystem::OpenFile(I_Path, EFileMode::Read | EFileMode::Binary);
        if (!File || !File->IsOpen())
        {
            LOG_ERROR("Failed to open font file: {}", I_Path);
            return nullptr;
        }
        TArray<FByte> FileBytes = File->ReadAll();
        if (FileBytes.IsEmpty())
        {
            LOG_ERROR("Failed to read font file or empty: {}", I_Path);
            return nullptr;
        }

        FFreeType::FFace Face{nullptr};
        TArray<FByte> FontData;
        const TOptional<FFontFaceInfo> InfoOpt = FFreeType::Load(FileBytes, I_FaceIndex, Face, FontData);
        if (!InfoOpt.HasValue() || Face == nullptr)
        {
            LOG_ERROR("Failed to load font from: {}", I_Path);
            return nullptr;
        }
        if (I_PixelSize > 0 && !FFreeType::SetPixelSizes(Face, I_PixelSize))
        {
            FFreeType::DoneFace(Face);
            LOG_ERROR("Failed to set font pixel size {} for: {}", I_PixelSize, I_Path);
            return nullptr;
        }
        FFont Font(std::move(FontData), InfoOpt.GetValue());
        auto NewFace = MakeShared<FFontAsset>(std::move(Font), Face);
        if (!AssetCache.Store(CacheKey, Cast<IAsset>(NewFace)))
        { LOG_WARN("Failed to store font {} (face {}, size {}) to asset cache!", I_Path, I_FaceIndex, I_PixelSize); }
        return NewFace;
    }

    TSharedPtr<FImageAsset> FAssetHub::
    LoadImageFromCache(const FName& I_Name)
    {
        auto W = AssetCache.Find(I_Name);
        if (W.IsExpired()) return nullptr;
        auto S = W.Lock();
        if (!S) return nullptr;
        return Cast<FImageAsset>(S);
    }

    TSharedPtr<FShaderAsset> FAssetHub::
    LoadShaderFromCache(const FName& I_Name)
    {
        auto W = AssetCache.Find(I_Name);
        if (W.IsExpired()) return nullptr;
        auto S = W.Lock();
        if (!S) return nullptr;
        return Cast<FShaderAsset>(S);
    }

    TSharedPtr<FFontAsset> FAssetHub::
    LoadFontFromCache(const FName& I_Name)
    {
        auto W = AssetCache.Find(I_Name);
        if (W.IsExpired()) return nullptr;
        auto S = W.Lock();
        if (!S) return nullptr;
        return Cast<FFontAsset>(S);
    }
}