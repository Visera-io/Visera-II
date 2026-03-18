module;
#include <Visera-Platform.hpp>
export module Visera.Platform;
#define VISERA_MODULE_NAME "Platform"
// Device enums (key, key state, mouse button, etc.) come from Interface.Device so Runtime.Input
// stays platform-agnostic; platform-specific code and casts stay in Platform layer.
export import Visera.Platform.Interface.Device;
#if defined(VISERA_ON_WINDOWS_SYSTEM)
import Visera.Platform.Windows;
export import Visera.Platform.Windows.Device;
#elif defined(VISERA_ON_APPLE_SYSTEM)
import Visera.Platform.MacOS;
export import Visera.Platform.MacOS.Device;
#else
import Visera.Platform.GLFW;
import Visera.Platform.Null;
export import Visera.Platform.GLFW.Device;
#endif
export import Visera.Core.Types.Path;
export import Visera.Core.Types.Text;
       import Visera.Core.Types.String;
       import Visera.Core.Types.Optional;
       import Visera.Core.Containers.Array;
       import Visera.Core.Meta.Cast;

export namespace Visera
{
#if defined(VISERA_ON_WINDOWS_SYSTEM)
    using EPlatformIOStatus       = EWindowsIOStatus;
    using FPlatformWindow         = FWindowsWindow;
    using FPlatformLibrary        = FWindowsLibrary;
    using FPlatformPath           = FWindowsPath;
    using FPlatformFileSystem     = FWindowsPlatformFileSystem;
#elif defined(VISERA_ON_APPLE_SYSTEM)
    using EPlatformIOStatus       = EMacOSIOStatus;
    using FPlatformWindow         = FMacOSWindow;
    using FPlatformLibrary        = FMacOSLibrary;
    using FPlatformPath           = FMacOSPath;
    using FPlatformFileSystem     = FMacOSPlatformFileSystem;
#else
#endif

    enum class EPlatform
    {
        Unknown,

        Windows,
        MacOS,
        GLFW,
        Null
    };

    class VISERA_PLATFORM_API FPlatform
    {
    public:
        [[nodiscard]] static inline TUniquePtr<FPlatformWindow>
        CreateWindow(FStringView I_Title, UInt32 I_Width, UInt32 I_Height) { return Cast<FPlatformWindow>(Get()->CreateWindow(I_Title, I_Width, I_Height)); }
        [[nodiscard]] static inline FPlatformPath
        MakePlatformPath(const FPath& I_Path) { return FPlatformPath(I_Path); }
        [[nodiscard]] static inline TSharedPtr<FPlatformLibrary>
        LoadLibrary(const FPath& I_Path) { return Cast<FPlatformLibrary>(Get()->LoadLibrary(MakePlatformPath(I_Path))); }
        [[nodiscard]] static const FPath&
        GetExecutableDirectory();
        [[nodiscard]] static const FPath&
        GetResourceDirectory();
        [[nodiscard]] static const FPath&
        GetFrameworkDirectory();
        [[nodiscard]] static const FPath&
        GetCacheDirectory();
        [[nodiscard]] static inline FPath
        GetLogsDirectory() { return Get()->GetLogsDirectory(); }
        [[nodiscard]] static inline Bool
        SetEnvironmentVariable(FStringView I_Variable, FStringView I_Value) { return Get()->SetEnvironmentVariable(I_Variable, I_Value); }
        [[nodiscard]] static inline TOptional<FString>
        GetEnvironmentVariable(FStringView I_Variable) { return Get()->GetEnvironmentVariable(I_Variable); }
        [[nodiscard]] static inline FUUID
        GenerateUUID() { return Get()->GenerateUUID(); }
        [[nodiscard]] static inline FStringView
        GetPlatformName() { return Get()->GetPlatformName(); }
        [[nodiscard]] static inline Bool
        IsPlatform(EPlatform I_Type) { return Get()->GetPlatformName() == NameFor(I_Type); }
        static inline void
        SetCurrentThreadName(FStringView I_Name) { Get()->SetCurrentThreadName(I_Name); }
        static inline void
        PollEvents() { Get()->PollEvents(); }
        static inline void
        WaitEvents() { Get()->WaitEvents(); }
        [[nodiscard]] static inline Bool
        ExistsFile(const FPath& I_Path) { return Get()->GetFileSystem()->ExistsFile(MakePlatformPath(I_Path)); }
        [[nodiscard]] static inline Bool
        ExistsDirectory(const FPath& I_Path) { return Get()->GetFileSystem()->ExistsDirectory(MakePlatformPath(I_Path)); }
        [[nodiscard]] static inline TArray<FPath>
        EnumerateFiles(const FPath& I_Directory, Bool I_bRecursive = False) { return Get()->GetFileSystem()->EnumerateFiles(MakePlatformPath(I_Directory), I_bRecursive); }
        [[nodiscard]] static inline EPlatformIOStatus
        CreateDirectories(const FPath& I_Path) { return static_cast<EPlatformIOStatus>(static_cast<UInt8>(Get()->GetFileSystem()->CreateDirectories(MakePlatformPath(I_Path)))); }
        [[nodiscard]] static inline TOptional<TArray<FByte>>
        ReadFile(const FPath& I_Path) { return Get()->GetFileSystem()->ReadFile(MakePlatformPath(I_Path)); }
        [[nodiscard]] static inline EPlatformIOStatus
        WriteFile(const FPath& I_Path, const void* I_Data, UInt64 I_Size) { return static_cast<EPlatformIOStatus>(static_cast<UInt8>(Get()->GetFileSystem()->WriteFile(MakePlatformPath(I_Path), I_Data, I_Size))); }
        [[nodiscard]] static inline EPlatformIOStatus
        DeleteFile(const FPath& I_Path) { return static_cast<EPlatformIOStatus>(static_cast<UInt8>(Get()->GetFileSystem()->DeleteFile(MakePlatformPath(I_Path)))); }
        [[nodiscard]] static inline EPlatformIOStatus
        ReplaceFile(const FPath& I_Source, const FPath& I_Target) { return static_cast<EPlatformIOStatus>(static_cast<UInt8>(Get()->GetFileSystem()->ReplaceFile(MakePlatformPath(I_Source), MakePlatformPath(I_Target)))); }
        [[nodiscard]] static inline EPlatformIOStatus
        AtomicWriteFile(const FPath& I_Path, const void* I_Data, UInt64 I_Size) { return static_cast<EPlatformIOStatus>(static_cast<UInt8>(Get()->GetFileSystem()->AtomicWriteFile(MakePlatformPath(I_Path), I_Data, I_Size))); }
        [[nodiscard]] static inline auto
        CreateTempFileNear(const FPath& I_Directory, const FPath& I_Prefix = FPath(".VTemp-")) { return Get()->GetFileSystem()->CreateTempFileNear(MakePlatformPath(I_Directory), MakePlatformPath(I_Prefix)); }
        /** Currently focused platform window for input. May be nullptr. */
        [[nodiscard]] static inline IPlatformWindow*
        GetFocusedWindow() { return Get()->GetFocusedWindow(); }

    private:
        static inline FStringView NameFor(EPlatform I_Type)
        {
            switch (I_Type)
            {
            case EPlatform::Windows: return "Windows";
            case EPlatform::MacOS:   return "MacOS";
            case EPlatform::GLFW:    return "GLFW";
            case EPlatform::Null:    return "Null";
            default:                 return "Unknown";
            }
        }

        static inline TUniqueRef<IPlatform>
        Get()
        {
            static auto Platform
#if defined(VISERA_ON_WINDOWS_SYSTEM)
            = MakeUnique<FWindowsPlatform>()
#elif defined(VISERA_ON_APPLE_SYSTEM)
            = MakeUnique<FMacOSPlatform>()
#else
            = FGLFWPlatform::IsSupported() ? MakeUnique<FGLFWPlatform>() : MakeUnique<FNullPlatform>()
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
            if (!Get()->GetFileSystem()->ExistsDirectory(MakePlatformPath(Cache.GetValue())))
            {
                const Int32 Err = Get()->GetFileSystem()->CreateDirectories(MakePlatformPath(Cache.GetValue()));
                VISERA_ASSERT(Err == 0);
            }
        }
        return Cache.GetValue();
    }
}