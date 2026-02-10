module;
#include <Visera-AssetHub.hpp>
export module Visera.Runtime.AssetHub;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
export import Visera.Core.Types.Path;
export import Visera.Runtime.AssetHub.Image;
export import Visera.Runtime.AssetHub.Shader;
export import Visera.Runtime.AssetHub.Font;
       import Visera.Runtime.AssetHub.Asset;
       import Visera.Core.Types.Pointer;
       import Visera.Core.Meta.Cast;
       import Visera.Core.Containers.Map;
       import Visera.Core.Containers.Cache;
       import Visera.Core.Containers.Array;
       import Visera.Core.Types.String;
       import Visera.Core.Types.JSON;
       import Visera.Core.Types.Optional;
       import Visera.Core.OS.Thread.Sync;
       import Visera.Core.OS.FileSystem;
       import Visera.Core.Image;
       import Visera.Core.Font;
       import Visera.Runtime.Global;

export namespace Visera
{
    class VISERA_RUNTIME_API FAssetHub : public IGlobalService
    {
    public:
        /** Load image; returns read-only FImageAsset (IAsset). Use SaveImage(view, path) to write. */
        [[nodiscard]] TSharedPtr<FImageAsset>
        LoadImage(const FPath& I_Path);
        /** Save image data to file. Takes FImageView2D for explicit region; copies to FImage and exports. */
        [[nodiscard]] Bool
        SaveImage(const FImageView2D& I_View, const FPath& I_Path);
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
        static constexpr UInt64 DefaultImageMB  = 64;
        static constexpr UInt64 DefaultShaderMB = 32;
        static constexpr UInt64 DefaultFontMB   = 16;

        static UInt64 GetCapacityMBFromConfig(const FJSON& I_Config, const char* I_TypeKey, UInt64 I_DefaultMB)
        {
            if (!I_Config.Contains("AssetHub")) { return I_DefaultMB; }
            const FJSON HubConfig = I_Config.GetObject("AssetHub");
            if (!HubConfig.Contains("CacheCapacity")) { return I_DefaultMB; }
            const FJSON CapConfig = HubConfig.GetObject("CacheCapacity");
            const Double Cap = CapConfig.GetNumber(I_TypeKey, static_cast<Double>(I_DefaultMB));
            return Cap > 0 ? static_cast<UInt64>(Cap) : I_DefaultMB;
        }

        static UInt64 GetAssetByteSize(const TSharedPtr<IAsset>& I_Ptr)
        { return I_Ptr ? I_Ptr->GetByteSize() : 0; }

        using FByteSizeFunc = UInt64(*)(const TSharedPtr<IAsset>&);
        using FHotCacheType = TLRUCache<FName, TSharedPtr<IAsset>, Policy::ByteWeighted<FByteSizeFunc>>;

        struct FCachePair
        {
            mutable FHotCacheType Hot;
            mutable TMap<FName, TWeakPtr<IAsset>> Cold;
            mutable FRWLock Lock;
        };
        FCachePair ImageCache;
        FCachePair ShaderCache;
        FCachePair FontCache;

        [[nodiscard]] TWeakPtr<IAsset> FindInCache(FCachePair& I_Cache, const FName& I_Key) const
        {
            FScopeWriteLock _{&I_Cache.Lock};
            if (TSharedPtr<IAsset>* Ptr = I_Cache.Hot.GetAndTouch(I_Key))
            { return TWeakPtr<IAsset>(*Ptr); }

            auto It = I_Cache.Cold.Find(I_Key);
            if (It != I_Cache.Cold.end())
            {
                TWeakPtr<IAsset>& W = It->second;
                if (TSharedPtr<IAsset> S = W.Lock())
                {
                    I_Cache.Hot.Put(I_Key, S);
                    return TWeakPtr<IAsset>(S);
                }
                I_Cache.Cold.Erase(I_Key);
            }
            return {};
        }

        void StoreInCache(FCachePair& I_Cache, const FName& I_Key, TSharedPtr<IAsset> I_Value)
        {
            FScopeWriteLock _{&I_Cache.Lock};
            I_Cache.Hot.Put(I_Key, I_Value);
            I_Cache.Cold[I_Key] = TWeakPtr<IAsset>(I_Value);
        }

    public:
        FAssetHub(FName I_Name, FServiceRegistry* I_Registry, const FJSON& I_Config)
            : IGlobalService(I_Name, I_Registry, I_Config)
            , ImageCache{
                FCachePair{
                    FHotCacheType(
                        GetCapacityMBFromConfig(I_Config, "Image", DefaultImageMB) * 1024 * 1024,
                        Policy::ByteWeighted<FByteSizeFunc>(&FAssetHub::GetAssetByteSize)),
                    {}, {}
                }}
            , ShaderCache{
                FCachePair{
                    FHotCacheType(
                        GetCapacityMBFromConfig(I_Config, "Shader", DefaultShaderMB) * 1024 * 1024,
                        Policy::ByteWeighted<FByteSizeFunc>(&FAssetHub::GetAssetByteSize)),
                    {}, {}
                }}
            , FontCache{
                FCachePair{
                    FHotCacheType(
                        GetCapacityMBFromConfig(I_Config, "Font", DefaultFontMB) * 1024 * 1024,
                        Policy::ByteWeighted<FByteSizeFunc>(&FAssetHub::GetAssetByteSize)),
                    {}, {}
                }}
        {
            Dependencies = { EName::Tasks };

            if (!OnBootstrap.TryBind([this] { return True; }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this] { return True; }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        }
    };

    TSharedPtr<FImageAsset> FAssetHub::
    LoadImage(const FPath& I_Path)
    {
        const FName PathName{I_Path.GetString()};
        if (auto W = FindInCache(ImageCache, PathName); !W.IsExpired())
        {
            LOG_DEBUG("LoadImage: {} (from cache).", I_Path);
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
        StoreInCache(ImageCache, PathName, Cast<IAsset>(NewAsset));
        LOG_DEBUG("LoadImage: {}.", I_Path);
        return NewAsset;
    }

    Bool FAssetHub::
    SaveImage(const FImageView2D& I_View, const FPath& I_Path)
    {
        const FImage ToSave{I_View, std::pmr::get_default_resource()};
        if (ToSave.GetWidth() == 0 || ToSave.GetHeight() == 0)
        {
            LOG_ERROR("Image view has invalid dimensions ({}x{}) for saving: {}",
                     ToSave.GetWidth(), ToSave.GetHeight(), I_Path);
            return False;
        }
        if (ToSave.GetPixelFormat() == EPixelFormat::Invalid)
        {
            LOG_ERROR("Image has invalid pixel format for saving: {}", I_Path);
            return False;
        }

        const EImageFormat TargetFormat = DetectImageFormat(I_Path);
        if (TargetFormat == EImageFormat::Invalid)
        {
            LOG_ERROR("Failed to detect image format from file extension for: {}", I_Path);
            return False;
        }

        TUniquePtr<IImageWrapper> Wrapper;
        switch (TargetFormat)
        {
        case EImageFormat::PNG:  Wrapper = MakeUnique<FPNGImageWrapper>(); break;
        case EImageFormat::EXR:  Wrapper = MakeUnique<FEXRImageWrapper>(); break;
        default:
            LOG_ERROR("Unsupported image format for saving: {}", I_Path);
            return False;
        }

        const Bool bSuccess = Wrapper->Export(ToSave, I_Path);
        if (bSuccess)
        { LOG_DEBUG("Successfully saved image to: {}", I_Path); }
        else
        { LOG_ERROR("Failed to save image to: {}", I_Path); }
        return bSuccess;
    }

    TSharedPtr<FShaderAsset> FAssetHub::
    LoadShader(const FPath& I_Path)
    {
        const FName PathName{I_Path.GetString()};
        if (auto W = FindInCache(ShaderCache, PathName); !W.IsExpired())
        {
            LOG_DEBUG("LoadShader: {} (from cache).", I_Path);
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
        StoreInCache(ShaderCache, PathName, Cast<IAsset>(NewShader));
        LOG_DEBUG("LoadShader: {}.", I_Path);
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

        if (auto W = FindInCache(FontCache, CacheKey); !W.IsExpired())
        {
            LOG_DEBUG("LoadFont: {} (face {}, size {}, from cache).", I_Path, I_FaceIndex, I_PixelSize);
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
        const auto InfoOpt = FFreeType::Load(FileBytes, I_FaceIndex, Face, FontData);
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
        StoreInCache(FontCache, CacheKey, Cast<IAsset>(NewFace));
        LOG_DEBUG("LoadFont: {} (face {}, size {}).", I_Path, I_FaceIndex, I_PixelSize);
        return NewFace;
    }

    TSharedPtr<FImageAsset> FAssetHub::
    LoadImageFromCache(const FName& I_Name)
    {
        auto W = FindInCache(ImageCache, I_Name);
        if (W.IsExpired()) return nullptr;
        auto S = W.Lock();
        if (!S) return nullptr;
        LOG_DEBUG("LoadImageFromCache: {}.", I_Name.GetName());
        return Cast<FImageAsset>(S);
    }

    TSharedPtr<FShaderAsset> FAssetHub::
    LoadShaderFromCache(const FName& I_Name)
    {
        auto W = FindInCache(ShaderCache, I_Name);
        if (W.IsExpired()) return nullptr;
        auto S = W.Lock();
        if (!S) return nullptr;
        LOG_DEBUG("LoadShaderFromCache: {}.", I_Name.GetName());
        return Cast<FShaderAsset>(S);
    }

    TSharedPtr<FFontAsset> FAssetHub::
    LoadFontFromCache(const FName& I_Name)
    {
        auto W = FindInCache(FontCache, I_Name);
        if (W.IsExpired()) return nullptr;
        auto S = W.Lock();
        if (!S) return nullptr;
        LOG_DEBUG("LoadFontFromCache: {}.", I_Name.GetName());
        return Cast<FFontAsset>(S);
    }
}