module;
#include <Visera-Platform.hpp>
export module Visera.Platform;
#define VISERA_MODULE_NAME "Platform"
#if defined(VISERA_ON_WINDOWS_SYSTEM)
import Visera.Platform.Windows;
#elif defined(VISERA_ON_APPLE_SYSTEM)
import Visera.Platform.MacOS;
#endif
export import Visera.Core.Types.Path;
       import Visera.Core.Types.Optional;
       import Visera.Core.Containers.Array;
       import Visera.Core.OS.FileSystem;

export namespace Visera
{
#if defined(VISERA_ON_WINDOWS_SYSTEM)
    using EPlatformIOStatus = EWindowsIOStatus;
#elif defined(VISERA_ON_APPLE_SYSTEM)
    using EPlatformIOStatus = EMacOSIOStatus;
#endif
    using FPlatformWindow   = IPlatformWindow;
    using FPlatformLibrary  = IPlatformLibrary;
#if defined(VISERA_ON_WINDOWS_SYSTEM)
    using FPlatformPath       = FWindowsPath;
    using FPlatformFileSystem = FWindowsPlatformFileSystem;
#elif defined(VISERA_ON_APPLE_SYSTEM)
    using FPlatformPath       = FMacOSPath;
    using FPlatformFileSystem = FMacOSPlatformFileSystem;
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
        static inline void
        SetCurrentThreadName(FStringView I_Name) { Get()->SetCurrentThreadName(I_Name); }

        [[nodiscard]] static inline Bool
        Exists(const FPath& I_Path) { return Get()->GetFileSystem().Exists(MakePlatformPath(I_Path)); }
        [[nodiscard]] static inline EPlatformIOStatus
        CreateDirectories(const FPath& I_Path) { return static_cast<EPlatformIOStatus>(static_cast<UInt8>(Get()->GetFileSystem().CreateDirectories(MakePlatformPath(I_Path)))); }
        [[nodiscard]] static inline TOptional<TArray<FByte>>
        ReadFile(const FPath& I_Path) { return Get()->GetFileSystem().ReadFile(MakePlatformPath(I_Path)); }
        [[nodiscard]] static inline EPlatformIOStatus
        WriteFile(const FPath& I_Path, const void* I_Data, UInt64 I_Size) { return static_cast<EPlatformIOStatus>(static_cast<UInt8>(Get()->GetFileSystem().WriteFile(MakePlatformPath(I_Path), I_Data, I_Size))); }
        [[nodiscard]] static inline EPlatformIOStatus
        ReplaceFile(const FPath& I_Source, const FPath& I_Target) { return static_cast<EPlatformIOStatus>(static_cast<UInt8>(Get()->GetFileSystem().ReplaceFile(MakePlatformPath(I_Source), MakePlatformPath(I_Target)))); }
        [[nodiscard]] static inline EPlatformIOStatus
        AtomicWriteFile(const FPath& I_Path, const void* I_Data, UInt64 I_Size) { return static_cast<EPlatformIOStatus>(static_cast<UInt8>(Get()->GetFileSystem().AtomicWriteFile(MakePlatformPath(I_Path), I_Data, I_Size))); }
        [[nodiscard]] static inline FTempFileResult
        CreateTempFileNear(const FPath& I_Directory, const FPath& I_Prefix = FPath(".VTemp-")) { return Get()->GetFileSystem().CreateTempFileNear(MakePlatformPath(I_Directory), MakePlatformPath(I_Prefix)); }

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
                VISERA_ASSERT(Error == EIOStatus::Success);
            }
        }
        return Cache.GetValue();
    }
}