module;
#include <Visera-AssetHub.hpp>
export module Visera.AssetHub;
#define VISERA_MODULE_NAME "AssetHub"
export import Visera.Core.Types.Path;
       import Visera.Core.Types.Map;
       import Visera.Core.Types.Optional;
       import Visera.Core.OS.Thread.Sync;
       import Visera.AssetHub.Image;
       import Visera.AssetHub.Shader;
       import Visera.Global;

export namespace Visera
{
    class VISERA_ASSETHUB_API FAssetHub : public IGlobalService
    {
    public:
        [[nodiscard]] TSharedPtr<FImage>
        LoadImage(const FPath& I_Path);
        /** Load .vshader from file. Requires AssetHub (and dependencies) to be registered. */
        [[nodiscard]] TOptional<FShader>
        LoadShader(const FPath& I_Path);

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
}