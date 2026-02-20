module;
#include <Visera-AssetHub.hpp>
export module Visera.Runtime.AssetHub;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
export import Visera.Core.Types.Path;
export import Visera.Runtime.AssetHub.Asset;
export import Visera.Runtime.AssetHub.Image;
export import Visera.Runtime.AssetHub.Shader;
export import Visera.Runtime.AssetHub.Font;
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
       import Visera.Runtime.Global;
       import Visera.Platform;

export namespace Visera
{
    class VISERA_RUNTIME_API FAssetHub : public IGlobalService
    {
    public:
        enum class ECacheClearTarget : UInt8
        {
            Image,
            Shader,
            Font,
            All,
        };
        /** Load image; returns read-only FImageAsset (IAsset). Use SaveImage(view, path) to write. */
        [[nodiscard]] TSharedPtr<FImageAsset>
        LoadImage(const FPath& I_Path, ELoadMode I_Mode = ELoadMode::Eager);
        /** Save image data to file. Takes FImageView2D for explicit region; copies to FImage and exports. */
        [[nodiscard]] Bool
        SaveImage(const FImageView2D& I_View, const FPath& I_Path, ESaveMode I_Mode = ESaveMode::AtomicReplace);
        /** Load .vshader from file. Returns read-only asset (IAsset). */
        [[nodiscard]] TSharedPtr<FShaderAsset>
        LoadShader(const FPath& I_Path, ELoadMode I_Mode = ELoadMode::Eager);
        /** Save shader data to .vshader file (pure data FShader; use FShader::Write* for custom serialization). */
        [[nodiscard]] Bool
        SaveShader(const FShader& I_Shader, const FPath& I_Path, ESaveMode I_Mode = ESaveMode::AtomicReplace);
        /** Load font face from file. Optional I_PixelSize: when > 0, size is set at load time (cached per path+face+size). Returns read-only FFontAsset (IAsset). */
        [[nodiscard]] TSharedPtr<FFontAsset>
        LoadFont(const FPath& I_Path, Int32 I_FaceIndex = 0, UInt32 I_PixelSize = 0, ELoadMode I_Mode = ELoadMode::Eager);

        /** Get cached image by name (same name as used when loaded by path, e.g. FName(I_Path.GetString())). Returns nullptr if not found or cast fails. */
        [[nodiscard]] TSharedPtr<FImageAsset>
        LoadImageFromCache(const FName& I_Name);
        /** Get cached shader by name. Returns nullptr if not found or cast fails. */
        [[nodiscard]] TSharedPtr<FShaderAsset>
        LoadShaderFromCache(const FName& I_Name);
        /** Get cached font by name (font cache name is path_faceIndex_pixelSize). Returns nullptr if not found or cast fails. */
        [[nodiscard]] TSharedPtr<FFontAsset>
        LoadFontFromCache(const FName& I_Name);
        /** Manually clear AssetHub caches. Default clears all cache tiers (hot+cold). */
        void
        ClearCache(ECacheClearTarget I_Target = ECacheClearTarget::All);

    private:
        static constexpr UInt64 DefaultImageMB  = 64;
        static constexpr UInt64 DefaultShaderMB = 32;
        static constexpr UInt64 DefaultFontMB   = 16;

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
            Font,
        };
        PROFILING_ONLY_FIELD(
        struct FProfilingMetrics
        {
            UInt64 LoadImageCalls {0};
            UInt64 LoadShaderCalls {0};
            UInt64 LoadFontCalls {0};

            UInt64 LoadImageFromCacheCalls {0};
            UInt64 LoadShaderFromCacheCalls {0};
            UInt64 LoadFontFromCacheCalls {0};

            UInt64 CacheHitHotImage {0};
            UInt64 CacheHitHotShader {0};
            UInt64 CacheHitHotFont {0};
            UInt64 CachePromoteColdImage {0};
            UInt64 CachePromoteColdShader {0};
            UInt64 CachePromoteColdFont {0};
            UInt64 CacheMissImage {0};
            UInt64 CacheMissShader {0};
            UInt64 CacheMissFont {0};
            UInt64 CacheColdExpiredPrunedImage {0};
            UInt64 CacheColdExpiredPrunedShader {0};
            UInt64 CacheColdExpiredPrunedFont {0};

            UInt64 StoreImageCalls {0};
            UInt64 StoreShaderCalls {0};
            UInt64 StoreFontCalls {0};

            UInt64 PeakHotEntriesImage {0};
            UInt64 PeakHotEntriesShader {0};
            UInt64 PeakHotEntriesFont {0};
            UInt64 PeakColdEntriesImage {0};
            UInt64 PeakColdEntriesShader {0};
            UInt64 PeakColdEntriesFont {0};
            UInt64 PeakHotWeightBytesImage {0};
            UInt64 PeakHotWeightBytesShader {0};
            UInt64 PeakHotWeightBytesFont {0};

            UInt64 SaveImageCalls {0};
            UInt64 SaveImageSuccess {0};
            UInt64 SaveShaderCalls {0};
            UInt64 SaveShaderSuccess {0};

            UInt64 CacheClearCalls {0};
            UInt64 CacheClearImageCalls {0};
            UInt64 CacheClearShaderCalls {0};
            UInt64 CacheClearFontCalls {0};
        } ProfilingMetrics {};
        );

        TUniquePtr<FCachePair> ImageCache;
        TUniquePtr<FCachePair> ShaderCache;
        TUniquePtr<FCachePair> FontCache;

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
            case ECacheKind::Font:
                if (HotEntries > ProfilingMetrics.PeakHotEntriesFont) { ProfilingMetrics.PeakHotEntriesFont = HotEntries; }
                if (ColdEntries > ProfilingMetrics.PeakColdEntriesFont) { ProfilingMetrics.PeakColdEntriesFont = ColdEntries; }
                if (HotWeightBytes > ProfilingMetrics.PeakHotWeightBytesFont) { ProfilingMetrics.PeakHotWeightBytesFont = HotWeightBytes; }
                break;
            default: break;
            }
            );
        }

        [[nodiscard]] TWeakPtr<IAsset> FindInCache(FCachePair& I_Cache, const FName& I_Key, ECacheKind I_Kind)
        {
            // [NOTE]:
            // - We take a write lock here because LRU "GetAndTouch" mutates the cache
            //   (moves the entry to MRU). A two-phase read->write approach would add
            //   complexity (tokens/generation, re-checks) with little benefit for our
            //   workload where cache hits are frequent and the critical section is small.
            // - If this becomes a contention hotspot, consider a two-phase Peek+Touch or
            //   an approximate-LRU scheme.
            FScopeWriteLock _{&I_Cache.Lock};

            if (TSharedPtr<IAsset>* Ptr = I_Cache.Hot.GetAndTouch(I_Key))
            {
                PROFILING_ONLY_FIELD(
                switch (I_Kind)
                {
                case ECacheKind::Image:  ++ProfilingMetrics.CacheHitHotImage; break;
                case ECacheKind::Shader: ++ProfilingMetrics.CacheHitHotShader; break;
                case ECacheKind::Font:   ++ProfilingMetrics.CacheHitHotFont; break;
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
                    case ECacheKind::Font:   ++ProfilingMetrics.CachePromoteColdFont; break;
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
                case ECacheKind::Font:   ++ProfilingMetrics.CacheColdExpiredPrunedFont; break;
                default: break;
                }
                );
            }
            PROFILING_ONLY_FIELD(
            switch (I_Kind)
            {
            case ECacheKind::Image:  ++ProfilingMetrics.CacheMissImage; break;
            case ECacheKind::Shader: ++ProfilingMetrics.CacheMissShader; break;
            case ECacheKind::Font:   ++ProfilingMetrics.CacheMissFont; break;
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
            case ECacheKind::Font:   ++ProfilingMetrics.StoreFontCalls; break;
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

    public:
        FAssetHub(FName I_Name, FServiceRegistry* I_Registry, const FJSON& I_Config)
            : IGlobalService(I_Name, I_Registry, I_Config)
        {
            auto GetCapMB = [&](const auto& I_Path, UInt64 I_Default) -> UInt64
            {
                const UInt64 V = I_Config.GetNumber(I_Path, static_cast<UInt64>(I_Default));
                return V > 0 ? V * 1024 * 1024 : I_Default * 1024 * 1024;
            };
            auto MakeCache = [](UInt64 I_CapBytes)
            {
                return TUniquePtr<FCachePair>(new FCachePair(
                    FHotCacheType(I_CapBytes, Policy::ByteWeighted<FByteSizeFunc>(&FAssetHub::GetAssetByteSize)),
                    {}, {}));
            };
            ImageCache  = MakeCache(GetCapMB(TJSONRoute<"AssetHub.CacheCapacityMB.Image">(),  DefaultImageMB));
            ShaderCache = MakeCache(GetCapMB(TJSONRoute<"AssetHub.CacheCapacityMB.Shader">(), DefaultShaderMB));
            FontCache   = MakeCache(GetCapMB(TJSONRoute<"AssetHub.CacheCapacityMB.Font">(),   DefaultFontMB));

            Dependencies = { EName::Tasks };

            if (!OnBootstrap.TryBind([this] { return True; }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                PROFILING_ONLY_FIELD(
                LOG_INFO("({}) [Profiling] AssetHub loads: image={}, shader={}, font={}; cache_loads: image={}, shader={}, font={}.",
                    GetRuntimeName(),
                    ProfilingMetrics.LoadImageCalls,
                    ProfilingMetrics.LoadShaderCalls,
                    ProfilingMetrics.LoadFontCalls,
                    ProfilingMetrics.LoadImageFromCacheCalls,
                    ProfilingMetrics.LoadShaderFromCacheCalls,
                    ProfilingMetrics.LoadFontFromCacheCalls);
                LOG_INFO("({}) [Profiling] AssetHub cache hits: hot(I={},S={},F={}) promote(I={},S={},F={}) miss(I={},S={},F={}) pruned(I={},S={},F={}).",
                    GetRuntimeName(),
                    ProfilingMetrics.CacheHitHotImage,
                    ProfilingMetrics.CacheHitHotShader,
                    ProfilingMetrics.CacheHitHotFont,
                    ProfilingMetrics.CachePromoteColdImage,
                    ProfilingMetrics.CachePromoteColdShader,
                    ProfilingMetrics.CachePromoteColdFont,
                    ProfilingMetrics.CacheMissImage,
                    ProfilingMetrics.CacheMissShader,
                    ProfilingMetrics.CacheMissFont,
                    ProfilingMetrics.CacheColdExpiredPrunedImage,
                    ProfilingMetrics.CacheColdExpiredPrunedShader,
                    ProfilingMetrics.CacheColdExpiredPrunedFont);
                LOG_INFO("({}) [Profiling] AssetHub cache peaks: hot_entries(I={},S={},F={}) cold_entries(I={},S={},F={}) hot_weight_bytes(I={},S={},F={}).",
                    GetRuntimeName(),
                    ProfilingMetrics.PeakHotEntriesImage,
                    ProfilingMetrics.PeakHotEntriesShader,
                    ProfilingMetrics.PeakHotEntriesFont,
                    ProfilingMetrics.PeakColdEntriesImage,
                    ProfilingMetrics.PeakColdEntriesShader,
                    ProfilingMetrics.PeakColdEntriesFont,
                    ProfilingMetrics.PeakHotWeightBytesImage,
                    ProfilingMetrics.PeakHotWeightBytesShader,
                    ProfilingMetrics.PeakHotWeightBytesFont);
                LOG_INFO("({}) [Profiling] AssetHub stores: image={}, shader={}, font={}; saves: image {}/{} shader {}/{}.",
                    GetRuntimeName(),
                    ProfilingMetrics.StoreImageCalls,
                    ProfilingMetrics.StoreShaderCalls,
                    ProfilingMetrics.StoreFontCalls,
                    ProfilingMetrics.SaveImageSuccess,
                    ProfilingMetrics.SaveImageCalls,
                    ProfilingMetrics.SaveShaderSuccess,
                    ProfilingMetrics.SaveShaderCalls);
                LOG_INFO("({}) [Profiling] AssetHub cache clears: all_calls={}, image_calls={}, shader_calls={}, font_calls={}.",
                    GetRuntimeName(),
                    ProfilingMetrics.CacheClearCalls,
                    ProfilingMetrics.CacheClearImageCalls,
                    ProfilingMetrics.CacheClearShaderCalls,
                    ProfilingMetrics.CacheClearFontCalls);
                );
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        }
    };

    TSharedPtr<FImageAsset> FAssetHub::
    LoadImage(const FPath& I_Path, ELoadMode I_Mode)
    {
        PROFILING_ONLY_FIELD(++ProfilingMetrics.LoadImageCalls;);
        const FName PathName{I_Path.GetString()};
        if (auto W = FindInCache(*ImageCache, PathName, ECacheKind::Image); !W.IsExpired())
        {
            LOG_DEBUG("({}) LoadImage: {} (from cache).", GetRuntimeName(), I_Path);
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
        StoreInCache(*ImageCache, PathName, Cast<IAsset>(NewAsset), ECacheKind::Image);
        LOG_DEBUG("({}) LoadImage: {}.", GetRuntimeName(), I_Path);
        return NewAsset;
    }

    Bool FAssetHub::
    SaveImage(const FImageView2D& I_View, const FPath& I_Path, ESaveMode I_Mode)
    {
        PROFILING_ONLY_FIELD(++ProfilingMetrics.SaveImageCalls;);
        const FImage ToSave{I_View, Memory::GetDefaultResource()};
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

        Bool bSuccess = False;
        if (I_Mode == ESaveMode::AtomicReplace)
        {
            const FPath Dir = I_Path.GetParent().HasValue() ? *I_Path.GetParent() : FPath(".");
            auto [TempFile, TempPathPtr] = FPlatform::CreateTempFileNear(Dir);
            if (!TempPathPtr)
            {
                LOG_ERROR("Failed to create temp file for atomic save: {}", I_Path);
                return False;
            }
            const FPath TempPath = TempPathPtr->ToPath();
            TempFile.Reset();
            if (!Wrapper->Export(ToSave, TempPath))
            {
                LOG_ERROR("Failed to export image to temp: {}", I_Path);
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
            LOG_DEBUG("({}) Successfully saved image to: {}", GetRuntimeName(), I_Path);
        }
        else
        { LOG_ERROR("Failed to save image to: {}", I_Path); }
        return bSuccess;
    }

    TSharedPtr<FShaderAsset> FAssetHub::
    LoadShader(const FPath& I_Path, ELoadMode I_Mode)
    {
        PROFILING_ONLY_FIELD(++ProfilingMetrics.LoadShaderCalls;);
        const FName PathName{I_Path.GetString()};
        if (auto W = FindInCache(*ShaderCache, PathName, ECacheKind::Shader); !W.IsExpired())
        {
            LOG_DEBUG("({}) LoadShader: {} (from cache).", GetRuntimeName(), I_Path);
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
        StoreInCache(*ShaderCache, PathName, Cast<IAsset>(NewShader), ECacheKind::Shader);
        LOG_DEBUG("({}) LoadShader: {}.", GetRuntimeName(), I_Path);
        return NewShader;
    }

    Bool FAssetHub::
    SaveShader(const FShader& I_Shader, const FPath& I_Path, ESaveMode I_Mode)
    {
        PROFILING_ONLY_FIELD(++ProfilingMetrics.SaveShaderCalls;);
        const Bool Saved = WriteShaderToFile(I_Shader, I_Path, I_Mode);
        PROFILING_ONLY_FIELD(if (Saved) { ++ProfilingMetrics.SaveShaderSuccess; });
        return Saved;
    }

    TSharedPtr<FFontAsset> FAssetHub::
    LoadFont(const FPath& I_Path, Int32 I_FaceIndex, UInt32 I_PixelSize, ELoadMode I_Mode)
    {
        PROFILING_ONLY_FIELD(++ProfilingMetrics.LoadFontCalls;);
        const FString CacheKeyStr = FString::Format("{}_{}_{}", I_Path.GetString(), I_FaceIndex, I_PixelSize);
        const FName CacheKey{CacheKeyStr};

        if (auto W = FindInCache(*FontCache, CacheKey, ECacheKind::Font); !W.IsExpired())
        {
            LOG_DEBUG("({}) LoadFont: {} (face {}, size {}, from cache).", GetRuntimeName(), I_Path, I_FaceIndex, I_PixelSize);
            return Cast<FFontAsset>(W.Lock());
        }

        auto FileBytesOpt = FPlatform::ReadFile(I_Path);
        if (!FileBytesOpt.HasValue() || FileBytesOpt->IsEmpty())
        {
            LOG_ERROR("Failed to read font file or empty: {}", I_Path);
            return nullptr;
        }

        FFreeType::FFace Face{nullptr};
        TArray<FByte> FontData;
        const auto InfoOpt = FFreeType::Load(FileBytesOpt.GetValue(), I_FaceIndex, Face, FontData);
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
        StoreInCache(*FontCache, CacheKey, Cast<IAsset>(NewFace), ECacheKind::Font);
        LOG_DEBUG("({}) LoadFont: {} (face {}, size {}).", GetRuntimeName(), I_Path, I_FaceIndex, I_PixelSize);
        return NewFace;
    }

    TSharedPtr<FImageAsset> FAssetHub::
    LoadImageFromCache(const FName& I_Name)
    {
        PROFILING_ONLY_FIELD(++ProfilingMetrics.LoadImageFromCacheCalls;);
        auto W = FindInCache(*ImageCache, I_Name, ECacheKind::Image);
        if (W.IsExpired()) return nullptr;
        auto S = W.Lock();
        if (!S) return nullptr;
        LOG_DEBUG("({}) LoadImageFromCache: {}.", GetRuntimeName(), I_Name.GetNameString());
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
        LOG_DEBUG("({}) LoadShaderFromCache: {}.", GetRuntimeName(), I_Name.GetNameString());
        return Cast<FShaderAsset>(S);
    }

    TSharedPtr<FFontAsset> FAssetHub::
    LoadFontFromCache(const FName& I_Name)
    {
        PROFILING_ONLY_FIELD(++ProfilingMetrics.LoadFontFromCacheCalls;);
        auto W = FindInCache(*FontCache, I_Name, ECacheKind::Font);
        if (W.IsExpired()) return nullptr;
        auto S = W.Lock();
        if (!S) return nullptr;
        LOG_DEBUG("({}) LoadFontFromCache: {}.", GetRuntimeName(), I_Name.GetNameString());
        return Cast<FFontAsset>(S);
    }

    void FAssetHub::
    ClearCache(ECacheClearTarget I_Target)
    {
        PROFILING_ONLY_FIELD(++ProfilingMetrics.CacheClearCalls;);
        switch (I_Target)
        {
        case ECacheClearTarget::Image:
            ClearCachePair(*ImageCache);
            PROFILING_ONLY_FIELD(++ProfilingMetrics.CacheClearImageCalls;);
            LOG_INFO("({}) [Profiling] AssetHub cache cleared: Image.", GetRuntimeName());
            break;
        case ECacheClearTarget::Shader:
            ClearCachePair(*ShaderCache);
            PROFILING_ONLY_FIELD(++ProfilingMetrics.CacheClearShaderCalls;);
            LOG_INFO("({}) [Profiling] AssetHub cache cleared: Shader.", GetRuntimeName());
            break;
        case ECacheClearTarget::Font:
            ClearCachePair(*FontCache);
            PROFILING_ONLY_FIELD(++ProfilingMetrics.CacheClearFontCalls;);
            LOG_INFO("({}) [Profiling] AssetHub cache cleared: Font.", GetRuntimeName());
            break;
        case ECacheClearTarget::All:
        default:
            ClearCachePair(*ImageCache);
            ClearCachePair(*ShaderCache);
            ClearCachePair(*FontCache);
            PROFILING_ONLY_FIELD(
            ++ProfilingMetrics.CacheClearImageCalls;
            ++ProfilingMetrics.CacheClearShaderCalls;
            ++ProfilingMetrics.CacheClearFontCalls;
            );
            LOG_INFO("({}) [Profiling] AssetHub cache cleared: All.", GetRuntimeName());
            break;
        }
    }
}