module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.AssetHub;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
export import Visera.Runtime.AssetHub.VPath;
export import Visera.Runtime.AssetHub.Asset;
export import Visera.Runtime.AssetHub.Image;
export import Visera.Runtime.AssetHub.Shader;
export import Visera.Runtime.AssetHub.Font;
       import Visera.Core.Log;
       import Visera.Core.Types.Name;
       import Visera.Core.Types.Path;
       import Visera.Core.Types.Pointer;
       import Visera.Core.Meta.Cast;
       import Visera.Core.Containers.Map;
       import Visera.Core.Containers.Cache;
       import Visera.Core.Containers.Array;
       import Visera.Core.Types.String;
       import Visera.Core.Types.JSON;
       import Visera.Core.Types.Optional;
       import Visera.Core.OS.Thread.Sync;
       import Visera.Core.Image;
       import Visera.Core.OS.Memory;
       import Visera.Core.Font;
       import Visera.Platform;

export namespace Visera
{
    struct VISERA_RUNTIME_API FAssetHubCreateInfo
    {
        UInt64 CacheCapacityMBImage  = kAssetHubDefaultImageMB;
        UInt64 CacheCapacityMBShader = kAssetHubDefaultShaderMB;
    };

    class VISERA_RUNTIME_API FAssetHub
    {
    public:
        explicit FAssetHub(const FAssetHubCreateInfo& I_CreateInfo);
        ~FAssetHub();

    public:
        enum class ECacheClearTarget : UInt8
        {
            Image,
            Shader,
            All,
        };

        /** Resolve a VPath to a concrete filesystem FPath. */
        [[nodiscard]] FPath
        ResolvePath(const VPath& I_VirtualPath) const;

        [[nodiscard]] TSharedPtr<FImageAsset>
        LoadImage(const VPath& I_AssetPath, ELoadMode I_Mode = ELoadMode::Eager);
        [[nodiscard]] Bool
        SaveImage(const FImageView2D& I_View, const VPath& I_AssetPath, ESaveMode I_Mode = ESaveMode::AtomicReplace);
        [[nodiscard]] TSharedPtr<FShaderAsset>
        LoadShader(const VPath& I_AssetPath, ELoadMode I_Mode = ELoadMode::Eager);
        [[nodiscard]] Bool
        SaveShader(const FShader& I_Shader, const VPath& I_AssetPath, ESaveMode I_Mode = ESaveMode::AtomicReplace);
        /** Load font face from file. No caching; caller owns the returned TSharedPtr. */
        [[nodiscard]] TSharedPtr<FFontAsset>
        LoadFont(const VPath& I_AssetPath, Int32 I_FaceIndex = 0, UInt32 I_PixelSize = 0);

        [[nodiscard]] TSharedPtr<FImageAsset>
        LoadImageFromCache(const FName& I_Name);
        [[nodiscard]] TSharedPtr<FShaderAsset>
        LoadShaderFromCache(const FName& I_Name);
        void
        ClearCache(ECacheClearTarget I_Target = ECacheClearTarget::All);

    private:

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
        enum class ECacheKind : UInt8
        {
            Image,
            Shader,
        };
        PROFILING_ONLY_FIELD(
        struct FProfilingMetrics
        {
            UInt64 LoadImageCalls {0};
            UInt64 LoadShaderCalls {0};
            UInt64 LoadFontCalls {0};

            UInt64 LoadImageFromCacheCalls {0};
            UInt64 LoadShaderFromCacheCalls {0};

            UInt64 CacheHitHotImage {0};
            UInt64 CacheHitHotShader {0};
            UInt64 CachePromoteColdImage {0};
            UInt64 CachePromoteColdShader {0};
            UInt64 CacheMissImage {0};
            UInt64 CacheMissShader {0};
            UInt64 CacheColdExpiredPrunedImage {0};
            UInt64 CacheColdExpiredPrunedShader {0};

            UInt64 StoreImageCalls {0};
            UInt64 StoreShaderCalls {0};

            UInt64 PeakHotEntriesImage {0};
            UInt64 PeakHotEntriesShader {0};
            UInt64 PeakColdEntriesImage {0};
            UInt64 PeakColdEntriesShader {0};
            UInt64 PeakHotWeightBytesImage {0};
            UInt64 PeakHotWeightBytesShader {0};

            UInt64 SaveImageCalls {0};
            UInt64 SaveImageSuccess {0};
            UInt64 SaveShaderCalls {0};
            UInt64 SaveShaderSuccess {0};

            UInt64 CacheClearCalls {0};
            UInt64 CacheClearImageCalls {0};
            UInt64 CacheClearShaderCalls {0};
        } ProfilingMetrics {};
        );

        TUniquePtr<FCachePair> ImageCache;
        TUniquePtr<FCachePair> ShaderCache;

        mutable TMap<FName, FPath> ResolvedPathCache;
        mutable FRWLock            ResolvedPathLock;

        void UpdateCachePeaks(ECacheKind I_Kind, const FCachePair& I_Cache)
        {
            PROFILING_ONLY_FIELD(
            const UInt64 HotEntries = I_Cache.Hot.GetSize();
            const UInt64 ColdEntries = I_Cache.Cold.GetSize();
            const UInt64 HotWeightBytes = I_Cache.Hot.GetCurrentWeight();
            switch (I_Kind)
            {
            case ECacheKind::Image:
                if (HotEntries > ProfilingMetrics.PeakHotEntriesImage) { ProfilingMetrics.PeakHotEntriesImage = HotEntries; }
                if (ColdEntries > ProfilingMetrics.PeakColdEntriesImage) { ProfilingMetrics.PeakColdEntriesImage = ColdEntries; }
                if (HotWeightBytes > ProfilingMetrics.PeakHotWeightBytesImage) { ProfilingMetrics.PeakHotWeightBytesImage = HotWeightBytes; }
                break;
            case ECacheKind::Shader:
                if (HotEntries > ProfilingMetrics.PeakHotEntriesShader) { ProfilingMetrics.PeakHotEntriesShader = HotEntries; }
                if (ColdEntries > ProfilingMetrics.PeakColdEntriesShader) { ProfilingMetrics.PeakColdEntriesShader = ColdEntries; }
                if (HotWeightBytes > ProfilingMetrics.PeakHotWeightBytesShader) { ProfilingMetrics.PeakHotWeightBytesShader = HotWeightBytes; }
                break;
            default: break;
            }
            );
        }

        [[nodiscard]] TWeakPtr<IAsset> FindInCache(FCachePair& I_Cache, const FName& I_Key, ECacheKind I_Kind)
        {
            FScopeWriteLock _{&I_Cache.Lock};

            if (TSharedPtr<IAsset>* Ptr = I_Cache.Hot.GetAndTouch(I_Key))
            {
                PROFILING_ONLY_FIELD(
                switch (I_Kind)
                {
                case ECacheKind::Image:  ++ProfilingMetrics.CacheHitHotImage; break;
                case ECacheKind::Shader: ++ProfilingMetrics.CacheHitHotShader; break;
                default: break;
                }
                );
                UpdateCachePeaks(I_Kind, I_Cache);
                return TWeakPtr<IAsset>(*Ptr);
            }

            auto It = I_Cache.Cold.Find(I_Key);
            if (It != I_Cache.Cold.end())
            {
                TWeakPtr<IAsset>& W = It->second;
                if (TSharedPtr<IAsset> S = W.Lock())
                {
                    I_Cache.Hot.Put(I_Key, S);
                    PROFILING_ONLY_FIELD(
                    switch (I_Kind)
                    {
                    case ECacheKind::Image:  ++ProfilingMetrics.CachePromoteColdImage; break;
                    case ECacheKind::Shader: ++ProfilingMetrics.CachePromoteColdShader; break;
                    default: break;
                    }
                    );
                    UpdateCachePeaks(I_Kind, I_Cache);
                    return TWeakPtr<IAsset>(S);
                }
                I_Cache.Cold.Erase(It);
                PROFILING_ONLY_FIELD(
                switch (I_Kind)
                {
                case ECacheKind::Image:  ++ProfilingMetrics.CacheColdExpiredPrunedImage; break;
                case ECacheKind::Shader: ++ProfilingMetrics.CacheColdExpiredPrunedShader; break;
                default: break;
                }
                );
            }
            PROFILING_ONLY_FIELD(
            switch (I_Kind)
            {
            case ECacheKind::Image:  ++ProfilingMetrics.CacheMissImage; break;
            case ECacheKind::Shader: ++ProfilingMetrics.CacheMissShader; break;
            default: break;
            }
            );
            return {};
        }

        void StoreInCache(FCachePair& I_Cache, const FName& I_Key, TSharedPtr<IAsset> I_Value, ECacheKind I_Kind)
        {
            FScopeWriteLock _{&I_Cache.Lock};
            I_Cache.Hot.Put(I_Key, I_Value);
            I_Cache.Cold[I_Key] = TWeakPtr<IAsset>(I_Value);
            PROFILING_ONLY_FIELD(
            switch (I_Kind)
            {
            case ECacheKind::Image:  ++ProfilingMetrics.StoreImageCalls; break;
            case ECacheKind::Shader: ++ProfilingMetrics.StoreShaderCalls; break;
            default: break;
            }
            );
            UpdateCachePeaks(I_Kind, I_Cache);
        }

        void ClearCachePair(FCachePair& I_Cache)
        {
            FScopeWriteLock _{&I_Cache.Lock};
            I_Cache.Hot.Clear();
            I_Cache.Cold.Clear();
        }

        static auto MakeCache(UInt64 I_CapBytes)
        {
            return TUniquePtr<FCachePair>(new FCachePair(
                FHotCacheType(I_CapBytes, Policy::ByteWeighted<FByteSizeFunc>(&FAssetHub::GetAssetByteSize)),
                {}, {}));
        }
    };
    inline FAssetHub* GAssetHub = nullptr;

    // ── ResolvePath ──────────────────────────────────────────────────────

    FPath FAssetHub::
    ResolvePath(const VPath& I_VirtualPath) const
    {
        const FName Key = I_VirtualPath.GetName();
        {
            FScopeReadLock _{&ResolvedPathLock};
            auto It = ResolvedPathCache.Find(Key);
            if (It != ResolvedPathCache.end())
                return It->second;
        }

        const FPath Root = [&]() -> FPath
        {
            switch (I_VirtualPath.GetScheme())
            {
            case EAssetScheme::App:    return FPlatform::GetExecutableDirectory();
            case EAssetScheme::Assets: return FPlatform::GetResourceDirectory() / FPath{"Assets"};
            case EAssetScheme::User:   return FPlatform::GetUserDataDirectory();
            case EAssetScheme::Cache:  return FPlatform::GetCacheDirectory();
            }
            return FPath{};
        }();

        const FStringView Relative = I_VirtualPath.GetRelativePath();
        const FPath Resolved = FPath::Normalized(Root / FPath{FString{Relative}});
        {
            FScopeWriteLock _{&ResolvedPathLock};
            ResolvedPathCache[Key] = Resolved;
        }
        return Resolved;
    }

    // ── Constructor / Destructor ─────────────────────────────────────────

    FAssetHub::FAssetHub(const FAssetHubCreateInfo& I_CreateInfo)
    {
        UInt64 CapImage  = (I_CreateInfo.CacheCapacityMBImage  > 0) ? I_CreateInfo.CacheCapacityMBImage  * 1024 * 1024 : kAssetHubDefaultImageMB  * 1024 * 1024;
        UInt64 CapShader = (I_CreateInfo.CacheCapacityMBShader > 0) ? I_CreateInfo.CacheCapacityMBShader * 1024 * 1024 : kAssetHubDefaultShaderMB * 1024 * 1024;
        ImageCache  = MakeCache(CapImage);
        ShaderCache = MakeCache(CapShader);
    }

    FAssetHub::~FAssetHub()
    {
        PROFILING_ONLY_FIELD(
        LOG_INFO("[Profiling] AssetHub loads: image={}, shader={}, font={}; cache_loads: image={}, shader={}.",
            ProfilingMetrics.LoadImageCalls,
            ProfilingMetrics.LoadShaderCalls,
            ProfilingMetrics.LoadFontCalls,
            ProfilingMetrics.LoadImageFromCacheCalls,
            ProfilingMetrics.LoadShaderFromCacheCalls);
        LOG_INFO("[Profiling] AssetHub cache hits: hot(I={},S={}) promote(I={},S={}) miss(I={},S={}) pruned(I={},S={}).",
            ProfilingMetrics.CacheHitHotImage,
            ProfilingMetrics.CacheHitHotShader,
            ProfilingMetrics.CachePromoteColdImage,
            ProfilingMetrics.CachePromoteColdShader,
            ProfilingMetrics.CacheMissImage,
            ProfilingMetrics.CacheMissShader,
            ProfilingMetrics.CacheColdExpiredPrunedImage,
            ProfilingMetrics.CacheColdExpiredPrunedShader);
        LOG_INFO("[Profiling] AssetHub cache peaks: hot_entries(I={},S={}) cold_entries(I={},S={}) hot_weight_MB(I={:.2f},S={:.2f}).",
            ProfilingMetrics.PeakHotEntriesImage,
            ProfilingMetrics.PeakHotEntriesShader,
            ProfilingMetrics.PeakColdEntriesImage,
            ProfilingMetrics.PeakColdEntriesShader,
            ProfilingMetrics.PeakHotWeightBytesImage / (1024.0 * 1024.0),
            ProfilingMetrics.PeakHotWeightBytesShader / (1024.0 * 1024.0));
        LOG_INFO("[Profiling] AssetHub stores: image={}, shader={}; saves: image {}/{} shader {}/{}.",
            ProfilingMetrics.StoreImageCalls,
            ProfilingMetrics.StoreShaderCalls,
            ProfilingMetrics.SaveImageSuccess,
            ProfilingMetrics.SaveImageCalls,
            ProfilingMetrics.SaveShaderSuccess,
            ProfilingMetrics.SaveShaderCalls);
        LOG_INFO("[Profiling] AssetHub cache clears: all_calls={}, image_calls={}, shader_calls={}.",
            ProfilingMetrics.CacheClearCalls,
            ProfilingMetrics.CacheClearImageCalls,
            ProfilingMetrics.CacheClearShaderCalls);
        );
    }

    // ── LoadImage ────────────────────────────────────────────────────────

    TSharedPtr<FImageAsset> FAssetHub::
    LoadImage(const VPath& I_AssetPath, ELoadMode I_Mode)
    {
        PROFILING_ONLY_FIELD(++ProfilingMetrics.LoadImageCalls;);
        const FName PathName = I_AssetPath.GetName();
        if (auto W = FindInCache(*ImageCache, PathName, ECacheKind::Image); !W.IsExpired())
        {
            LOG_DEBUG("LoadImage: {} (from cache).", I_AssetPath);
            return Cast<FImageAsset>(W.Lock());
        }

        const FPath Resolved = ResolvePath(I_AssetPath);
        const EImageFormat Format = DetectImageFormat(Resolved);
        if (Format == EImageFormat::Invalid)
        {
            LOG_ERROR("Failed to detect image format for: {}", I_AssetPath);
            return nullptr;
        }

        TUniquePtr<IImageWrapper> Wrapper;
        switch (Format)
        {
        case EImageFormat::PNG:  Wrapper = MakeUnique<FPNGImageWrapper>(); break;
        case EImageFormat::EXR:  Wrapper = MakeUnique<FEXRImageWrapper>(); break;
        default:
            LOG_ERROR("Unsupported image format for: {}", I_AssetPath);
            return nullptr;
        }

        FImage NewImage = Wrapper->Import(Resolved);
        if (NewImage.GetWidth() == 0) return nullptr;

        auto NewAsset = MakeShared<FImageAsset>(std::move(NewImage));
        StoreInCache(*ImageCache, PathName, Cast<IAsset>(NewAsset), ECacheKind::Image);
        LOG_DEBUG("LoadImage: {}.", I_AssetPath);
        return NewAsset;
    }

    // ── SaveImage ────────────────────────────────────────────────────────

    Bool FAssetHub::
    SaveImage(const FImageView2D& I_View, const VPath& I_AssetPath, ESaveMode I_Mode)
    {
        PROFILING_ONLY_FIELD(++ProfilingMetrics.SaveImageCalls;);
        const FPath I_Path = ResolvePath(I_AssetPath);

        const FImage ToSave{I_View, Memory::GetDefaultResource()};
        if (ToSave.GetWidth() == 0 || ToSave.GetHeight() == 0)
        {
            LOG_ERROR("Image view has invalid dimensions ({}x{}) for saving: {}",
                     ToSave.GetWidth(), ToSave.GetHeight(), I_AssetPath);
            return False;
        }
        if (ToSave.GetPixelFormat() == EPixelFormat::Invalid)
        {
            LOG_ERROR("Image has invalid pixel format for saving: {}", I_AssetPath);
            return False;
        }

        const EImageFormat TargetFormat = DetectImageFormat(I_Path);
        if (TargetFormat == EImageFormat::Invalid)
        {
            LOG_ERROR("Failed to detect image format from file extension for: {}", I_AssetPath);
            return False;
        }

        TUniquePtr<IImageWrapper> Wrapper;
        switch (TargetFormat)
        {
        case EImageFormat::PNG:  Wrapper = MakeUnique<FPNGImageWrapper>(); break;
        case EImageFormat::EXR:  Wrapper = MakeUnique<FEXRImageWrapper>(); break;
        default:
            LOG_ERROR("Unsupported image format for saving: {}", I_AssetPath);
            return False;
        }

        Bool bSuccess = False;
        if (I_Mode == ESaveMode::AtomicReplace)
        {
            const FPath Dir = I_Path.GetParent().HasValue() ? *I_Path.GetParent() : FPath(".");
            auto [TempFile, TempPathPtr] = FPlatform::CreateTempFileNear(Dir);
            if (!TempPathPtr)
            {
                LOG_ERROR("Failed to create temp file for atomic save: {}", I_AssetPath);
                return False;
            }
            const FPath TempPath = TempPathPtr->ToPath();
            TempFile.Reset();
            if (!Wrapper->Export(ToSave, TempPath))
            {
                LOG_ERROR("Failed to export image to temp: {}", I_AssetPath);
                (void)FPlatform::DeleteFile(TempPath);
                return False;
            }
            const auto Status = FPlatform::ReplaceFile(TempPath, I_Path);
            bSuccess = (Status == EPlatformIOStatus::Success);
            if (!bSuccess)
            { (void)FPlatform::DeleteFile(TempPath); }
        }
        else
        {
            bSuccess = Wrapper->Export(ToSave, I_Path);
        }
        if (bSuccess)
        {
            PROFILING_ONLY_FIELD(++ProfilingMetrics.SaveImageSuccess;);
            LOG_DEBUG("Successfully saved image to: {}", I_AssetPath);
        }
        else
        { LOG_ERROR("Failed to save image to: {}", I_AssetPath); }
        return bSuccess;
    }

    // ── LoadShader ───────────────────────────────────────────────────────

    TSharedPtr<FShaderAsset> FAssetHub::
    LoadShader(const VPath& I_AssetPath, ELoadMode I_Mode)
    {
        PROFILING_ONLY_FIELD(++ProfilingMetrics.LoadShaderCalls;);
        const FName PathName = I_AssetPath.GetName();
        if (auto W = FindInCache(*ShaderCache, PathName, ECacheKind::Shader); !W.IsExpired())
        {
            LOG_DEBUG("LoadShader: {} (from cache).", I_AssetPath);
            return Cast<FShaderAsset>(W.Lock());
        }

        const FPath Resolved = ResolvePath(I_AssetPath);
        TArray<FByte> SPIRVChunk, ReflectionChunk;
        UInt32 Version = 0;
        if (!ReadShaderChunks(Resolved, Version, SPIRVChunk, ReflectionChunk) || SPIRVChunk.IsEmpty())
        { return nullptr; }
        FShader::FLayout Refl;
        if (ReflectionChunk.IsEmpty() || !DeserializeShaderReflection(Version, FStringView(reinterpret_cast<const char*>(ReflectionChunk.Data()), ReflectionChunk.GetSize()), Refl))
        { return nullptr; }
        if (Refl.EntryPoints.IsEmpty())
        { return nullptr; }
        auto NewShader = MakeShared<FShaderAsset>(FShader{std::move(SPIRVChunk), std::move(Refl)});
        StoreInCache(*ShaderCache, PathName, Cast<IAsset>(NewShader), ECacheKind::Shader);
        LOG_DEBUG("LoadShader: {}.", I_AssetPath);
        return NewShader;
    }

    // ── SaveShader ───────────────────────────────────────────────────────

    Bool FAssetHub::
    SaveShader(const FShader& I_Shader, const VPath& I_AssetPath, ESaveMode I_Mode)
    {
        PROFILING_ONLY_FIELD(++ProfilingMetrics.SaveShaderCalls;);
        const FPath Resolved = ResolvePath(I_AssetPath);
        const Bool Saved = WriteShaderToFile(I_Shader, Resolved, I_Mode);
        PROFILING_ONLY_FIELD(if (Saved) { ++ProfilingMetrics.SaveShaderSuccess; });
        return Saved;
    }

    // ── LoadFont (no cache) ──────────────────────────────────────────────

    TSharedPtr<FFontAsset> FAssetHub::
    LoadFont(const VPath& I_AssetPath, Int32 I_FaceIndex, UInt32 I_PixelSize)
    {
        PROFILING_ONLY_FIELD(++ProfilingMetrics.LoadFontCalls;);
        const FPath Resolved = ResolvePath(I_AssetPath);

        auto FileBytesOpt = FPlatform::ReadFile(Resolved);
        if (!FileBytesOpt.HasValue() || FileBytesOpt->IsEmpty())
        {
            LOG_ERROR("Failed to read font file or empty: {}", I_AssetPath);
            return nullptr;
        }

        FFreeType::FFace Face{nullptr};
        TArray<FByte> FontData;
        const auto InfoOpt = FFreeType::Load(FileBytesOpt.GetValue(), I_FaceIndex, Face, FontData);
        if (!InfoOpt.HasValue() || Face == nullptr)
        {
            LOG_ERROR("Failed to load font from: {}", I_AssetPath);
            return nullptr;
        }
        if (I_PixelSize > 0 && !FFreeType::SetPixelSizes(Face, I_PixelSize))
        {
            FFreeType::DoneFace(Face);
            LOG_ERROR("Failed to set font pixel size {} for: {}", I_PixelSize, I_AssetPath);
            return nullptr;
        }
        FFont Font(std::move(FontData), InfoOpt.GetValue());
        auto NewFace = MakeShared<FFontAsset>(std::move(Font), Face);
        LOG_DEBUG("LoadFont: {} (face {}, size {}).", I_AssetPath, I_FaceIndex, I_PixelSize);
        return NewFace;
    }

    // ── Cache-only loads ─────────────────────────────────────────────────

    TSharedPtr<FImageAsset> FAssetHub::
    LoadImageFromCache(const FName& I_Name)
    {
        PROFILING_ONLY_FIELD(++ProfilingMetrics.LoadImageFromCacheCalls;);
        auto W = FindInCache(*ImageCache, I_Name, ECacheKind::Image);
        if (W.IsExpired()) return nullptr;
        auto S = W.Lock();
        if (!S) return nullptr;
        LOG_DEBUG("LoadImageFromCache: {}.", I_Name.GetNameString());
        return Cast<FImageAsset>(S);
    }

    TSharedPtr<FShaderAsset> FAssetHub::
    LoadShaderFromCache(const FName& I_Name)
    {
        PROFILING_ONLY_FIELD(++ProfilingMetrics.LoadShaderFromCacheCalls;);
        auto W = FindInCache(*ShaderCache, I_Name, ECacheKind::Shader);
        if (W.IsExpired()) return nullptr;
        auto S = W.Lock();
        if (!S) return nullptr;
        LOG_DEBUG("LoadShaderFromCache: {}.", I_Name.GetNameString());
        return Cast<FShaderAsset>(S);
    }

    // ── ClearCache ───────────────────────────────────────────────────────

    void FAssetHub::
    ClearCache(ECacheClearTarget I_Target)
    {
        PROFILING_ONLY_FIELD(++ProfilingMetrics.CacheClearCalls;);
        switch (I_Target)
        {
        case ECacheClearTarget::Image:
            ClearCachePair(*ImageCache);
            PROFILING_ONLY_FIELD(++ProfilingMetrics.CacheClearImageCalls;);
            LOG_INFO("[Profiling] AssetHub cache cleared: Image.");
            break;
        case ECacheClearTarget::Shader:
            ClearCachePair(*ShaderCache);
            PROFILING_ONLY_FIELD(++ProfilingMetrics.CacheClearShaderCalls;);
            LOG_INFO("[Profiling] AssetHub cache cleared: Shader.");
            break;
        case ECacheClearTarget::All:
        default:
            ClearCachePair(*ImageCache);
            ClearCachePair(*ShaderCache);
            PROFILING_ONLY_FIELD(
            ++ProfilingMetrics.CacheClearImageCalls;
            ++ProfilingMetrics.CacheClearShaderCalls;
            );
            LOG_INFO("[Profiling] AssetHub cache cleared: All.");
            break;
        }
    }
}
