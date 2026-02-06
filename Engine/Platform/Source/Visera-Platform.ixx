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

export namespace Visera
{
    using FPlatformWindow  = IPlatformWindow;
    using FPlatformLibrary = IPlatformLibrary;

    class VISERA_PLATFORM_API FPlatform
    {
    public:
        [[nodiscard]] static inline TUniquePtr<FPlatformWindow>
        CreateWindow(FStringView I_Title, UInt32 I_Width, UInt32 I_Height) { return Get()->CreateWindow(I_Title, I_Width, I_Height); }
        [[nodiscard]] static inline TSharedPtr<FPlatformLibrary>
        LoadLibrary(const FPath& I_Path) { return Get()->LoadLibrary(I_Path); }
        [[nodiscard]] static inline const FPath&
        GetExecutableDirectory() { return Get()->GetExecutableDirectory(); }
        [[nodiscard]] static inline const FPath&
        GetResourceDirectory() { return Get()->GetResourceDirectory(); }
        [[nodiscard]] static inline const FPath&
        GetFrameworkDirectory() { return Get()->GetFrameworkDirectory(); }
        [[nodiscard]] static inline const FPath&
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

    const FPath& FPlatform::
    GetCacheDirectory()
    {
        static FPath CacheDirectory = Get()->GetResourceDirectory() / "Cache";
        if (!FFileSystem::Exists(CacheDirectory))
        {
            const auto Error = FFileSystem::CreateDirectory(CacheDirectory);
            VISERA_ASSERT(!Error);
        }
        return CacheDirectory;
    }
}