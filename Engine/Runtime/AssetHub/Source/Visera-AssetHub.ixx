module;
#include <Visera-AssetHub.hpp>
export module Visera.AssetHub;
#define VISERA_MODULE_NAME "AssetHub"
export import Visera.Core.Types.Path;
       import Visera.Core.Types.Map;
       import Visera.Core.Types.Array;
       import Visera.Core.Types.String;
       import Visera.Core.Types.Optional;
       import Visera.Core.OS.Thread.Sync;
       import Visera.AssetHub.Image;
       import Visera.AssetHub.Shader;
       import Visera.AssetHub.Font;
       import Visera.Global;

export namespace Visera
{
    class VISERA_ASSETHUB_API FAssetHub : public IGlobalService
    {
    public:
        [[nodiscard]] TSharedPtr<FImage>
        LoadImage(const FPath& I_Path);
        /** Save image to file. Automatically detects format from extension. */
        [[nodiscard]] Bool
        SaveImage(TSharedPtr<const FImage> I_Image, const FPath& I_Path);
        /** Load .vshader from file. Requires AssetHub (and dependencies) to be registered. */
        [[nodiscard]] TOptional<FShader>
        LoadShader(const FPath& I_Path);
        /** Load font face from file. */
        [[nodiscard]] TSharedPtr<FFont>
        LoadFont(const FPath& I_Path, Int32 I_FaceIndex = 0);

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
        TCache<FImage> ImageCache;
        TCache<FFont> FontCache;

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

    /**
     * Loads an image from a file path and creates an FImage.
     * Automatically detects the image format and uses the appropriate loader.
     * @param I_Path The path to the image file
     * @return A shared pointer to the loaded FImage, or nullptr on failure
     */
    TSharedPtr<FImage> FAssetHub::
    LoadImage(const FPath& I_Path)
    {
        FName PathName = FName{I_Path.GetUTF8Path()};
        if (auto Cache = ImageCache.Find(PathName); !Cache.IsExpired())
        {
            LOG_TRACE("Get {} from cache.", I_Path);
            return Cache.Lock();
        }

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

        case EImageFormat::EXR:
            Wrapper = MakeUnique<FEXRImageWrapper>();
            break;
        
        default:
            LOG_ERROR("Unsupported image format for: {}", I_Path);
            return nullptr;
        }

        auto NewImage = Wrapper->Import(I_Path);
        if (!ImageCache.Store(PathName, NewImage))
        { LOG_WARN("Failed to store the {} to image cache!", I_Path); }

        return NewImage;
    }

    Bool FAssetHub::
    SaveImage(TSharedPtr<const FImage> I_Image, const FPath& I_Path)
    {
        // Validate image
        if (!I_Image)
        {
            LOG_ERROR("Invalid image for saving: {}", I_Path);
            return False;
        }

        if (I_Image->GetWidth() == 0 || I_Image->GetHeight() == 0)
        {
            LOG_ERROR("Image has invalid dimensions ({}x{}) for saving: {}", 
                     I_Image->GetWidth(), I_Image->GetHeight(), I_Path);
            return False;
        }

        if (I_Image->GetPixelFormat() == EPixelFormat::Invalid)
        {
            LOG_ERROR("Image has invalid pixel format for saving: {}", I_Path);
            return False;
        }

        // Auto-detect format based on pixel format
        // EXR for float formats, PNG for integer formats
        const EPixelFormat PixelFormat = I_Image->GetPixelFormat();
        const Bool IsFloatFormat = I_Image->IsFloatFormat();
        
        EImageFormat TargetFormat = EImageFormat::Invalid;
        
        if (IsFloatFormat)
        {
            // Float formats -> EXR
            TargetFormat = EImageFormat::EXR;
        }
        else
        {
            // Integer formats -> PNG
            TargetFormat = EImageFormat::PNG;
        }

        // Verify file extension matches the target format
        const EImageFormat ExtensionFormat = DetectImageFormat(I_Path);
        if (ExtensionFormat != TargetFormat)
        {
            LOG_WARN("File extension does not match image format. Image format: {}, Extension format: {}. Using image format.", 
                    static_cast<Int32>(TargetFormat), static_cast<Int32>(ExtensionFormat));
        }

        // Create appropriate wrapper and save image
        TUniquePtr<IImageWrapper> Wrapper;
        switch (TargetFormat)
        {
        case EImageFormat::PNG:
            Wrapper = MakeUnique<FPNGImageWrapper>();
            break;

        case EImageFormat::EXR:
            Wrapper = MakeUnique<FEXRImageWrapper>();
            break;
        
        default:
            LOG_ERROR("Unsupported pixel format for saving: {} (format: {})", 
                     I_Path, static_cast<Int32>(PixelFormat));
            return False;
        }

        return Wrapper->Export(I_Image, I_Path);
    }

    TOptional<FShader> FAssetHub::
    LoadShader(const FPath& I_Path)
    {
        TArray<FByte> SPIRVChunk, ReflectionChunk;
        UInt32 Version = 0;
        if (!ReadShaderChunks(I_Path, Version, SPIRVChunk, ReflectionChunk) || SPIRVChunk.IsEmpty())
        { return NullOpt; }
        FShaderReflection Refl;
        if (ReflectionChunk.IsEmpty() || !DeserializeShaderReflection(Version, FStringView(reinterpret_cast<const char*>(ReflectionChunk.Data()), ReflectionChunk.GetSize()), Refl))
        { return NullOpt; }
        if (Refl.EntryPoints.IsEmpty())
        { return NullOpt; }
        FShader Shader;
        Shader.SPIRV = std::move(SPIRVChunk);
        Shader.Reflection = std::move(Refl);
        return TOptional<FShader>(std::move(Shader));
    }

    /**
     * Loads a font face from a file path and creates an FFont.
     * The font face can be used for rendering glyphs and generating MSDF atlas.
     * @param I_Path The path to the font file
     * @param I_FaceIndex Face index in the font file (0 for single-face fonts)
     * @return A shared pointer to the loaded FFont, or nullptr on failure
     */
    TSharedPtr<FFont> FAssetHub::
    LoadFont(const FPath& I_Path, Int32 I_FaceIndex)
    {
        // Create cache key from path and face index
        const FString CacheKeyStr = FString::Format("{}_{}", I_Path.GetUTF8Path(), I_FaceIndex);
        const FName CacheKey{CacheKeyStr};

        if (auto Cache = FontCache.Find(CacheKey); !Cache.IsExpired())
        {
            LOG_TRACE("Get font {} (face {}) from cache.", I_Path, I_FaceIndex);
            return Cache.Lock();
        }

        auto NewFace = MakeShared<FFont>();
        if (!NewFace->LoadFromFile(I_Path, I_FaceIndex))
        {
            LOG_ERROR("Failed to load font from: {}", I_Path);
            return nullptr;
        }

        if (!FontCache.Store(CacheKey, NewFace))
        { LOG_WARN("Failed to store the font {} (face {}) to font cache!", I_Path, I_FaceIndex); }

        return NewFace;
    }
}