module;
#include <Visera-Platform.hpp>
export module Visera.Platform;
#define VISERA_MODULE_NAME "Platform"
#if defined(VISERA_ON_WINDOWS_SYSTEM)
import Visera.Platform.Windows;
#elif defined(VISERA_ON_APPLE_SYSTEM)
import Visera.Platform.MacOS;
#endif
import Visera.Platform.Interface;
import Visera.Core.OS.FileSystem;
import Visera.Core.Types.Optional;

export namespace Visera
{
    using FPlatformWindow  = IPlatformWindow;
    using FPlatformLibrary = IPlatformLibrary;
#if defined(VISERA_ON_WINDOWS_SYSTEM)
    using FPlatformPath = FWindowsPath;
#elif defined(VISERA_ON_APPLE_SYSTEM)
    using FPlatformPath = FMacOSPath;
#endif

    class VISERA_PLATFORM_API FPlatform
    {
    public:
        [[nodiscard]] static inline TUniquePtr<FPlatformWindow>
        CreateWindow(FStringView I_Title, UInt32 I_Width, UInt32 I_Height) { return Get()->CreateWindow(I_Title, I_Width, I_Height); }
        [[nodiscard]] static inline FPlatformPath
        MakePlatformPath(const FPath& I_Path) { return FPlatformPath(I_Path); }
        [[nodiscard]] static inline TSharedPtr<FPlatformLibrary>
        LoadLibrary(const FPath& I_Path) { return Get()->LoadLibrary(MakePlatformPath(I_Path)); }
        [[nodiscard]] static const FPath&
        GetExecutableDirectory();
        [[nodiscard]] static const FPath&
        GetResourceDirectory();
        [[nodiscard]] static const FPath&
        GetFrameworkDirectory();
        [[nodiscard]] static const FPath&
        GetCacheDirectory();
        [[nodiscard]] static inline Bool
        SetEnvironmentVariable(FStringView I_Variable, FStringView I_Value) { return Get()->SetEnvironmentVariable(I_Variable, I_Value); }
        [[nodiscard]] static inline FUUID
        GenerateUUID() { return Get()->GenerateUUID(); }
        [[nodiscard]] static inline EPlatform
        GetType() { return Get()->GetType(); }

    private:
        static inline TUniqueRef<IPlatform>
        Get()
        {
            static auto Platform
#if defined(VISERA_ON_WINDOWS_SYSTEM)
            = MakeUnique<FWindowsPlatform>()
#elif defined(VISERA_ON_APPLE_SYSTEM)
            = MakeUnique<FMacOSPlatform>()
#endif
            ;
            return Platform;
        }
    };

    const FPath& FPlatform::GetExecutableDirectory()
    {
        static TOptional<FPath> Cache;
        if (!Cache.HasValue())
        {
            if (TUniquePtr<IPlatformPath> P = Get()->GetExecutableDirectory(); P)
                Cache = P->ToPath();
            else
                Cache = FPath("");
        }
        return Cache.GetValue();
    }

    const FPath& FPlatform::GetResourceDirectory()
    {
        static TOptional<FPath> Cache;
        if (!Cache.HasValue())
        {
            if (TUniquePtr<IPlatformPath> P = Get()->GetResourceDirectory(); P)
                Cache = P->ToPath();
            else
                Cache = FPath("");
        }
        return Cache.GetValue();
    }

    const FPath& FPlatform::GetFrameworkDirectory()
    {
        static TOptional<FPath> Cache;
        if (!Cache.HasValue())
        {
            if (TUniquePtr<IPlatformPath> P = Get()->GetFrameworkDirectory(); P)
                Cache = P->ToPath();
            else
                Cache = FPath("");
        }
        return Cache.GetValue();
    }

    const FPath& FPlatform::GetCacheDirectory()
    {
        static TOptional<FPath> Cache;
        if (!Cache.HasValue())
        {
            Cache = GetResourceDirectory() / FPath{"Cache"};
            if (!FFileSystem::Exists(Cache.GetValue()))
            {
                const auto Error = FFileSystem::CreateDirectory(Cache.GetValue());
                VISERA_ASSERT(Error == EIOError::None);
            }
        }
        return Cache.GetValue();
    }
}