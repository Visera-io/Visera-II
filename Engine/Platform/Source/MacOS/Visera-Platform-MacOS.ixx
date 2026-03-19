module;
#include <Visera-Platform.hpp>
#if defined(VISERA_ON_APPLE_SYSTEM)
#include <mach-o/dyld.h>
#include <pthread.h>
#include <uuid/uuid.h>
#endif
export module Visera.Platform.MacOS;
#define VISERA_MODULE_NAME "Platform.MacOS"
export import Visera.Platform.Interface;
export import Visera.Platform.MacOS.Path;
export import Visera.Platform.MacOS.Device;
export import Visera.Platform.MacOS.Window;
export import Visera.Platform.MacOS.Library;
export import Visera.Platform.MacOS.FileSystem;
       import Visera.Platform.GLFW;
       import Visera.Core.Types.Optional;
       import Visera.Core.Types.Path;
       import Visera.Core.Types.String;
       import Visera.Core.Log;

export namespace Visera
{
    class VISERA_PLATFORM_API FMacOSPlatform : public IPlatform
    {
    public:
        [[nodiscard]] TUniquePtr<IPlatformWindow>
        CreateWindow(FStringView I_Title, UInt32 I_Width, UInt32 I_Height) const override;
        [[nodiscard]] TSharedPtr<IPlatformLibrary>
        LoadLibrary(const IPlatformPath& I_Path) const override { return MakeShared<FMacOSLibrary>(I_Path); }
        [[nodiscard]] TUniquePtr<IPlatformPath>
        GetExecutableDirectory() const override;
        [[nodiscard]] TUniquePtr<IPlatformPath>
        GetResourceDirectory() const override;
        [[nodiscard]] TUniquePtr<IPlatformPath>
        GetFrameworkDirectory() const override;
        [[nodiscard]] TUniquePtr<IPlatformPath>
        GetUserDataDirectory() const override;
        [[nodiscard]] FPath
        GetLogsDirectory() const override;
        [[nodiscard]] Bool
        SetEnvironmentVariable(FStringView I_Variable, FStringView I_Value) const override;
        [[nodiscard]] TOptional<FString>
        GetEnvironmentVariable(FStringView I_Variable) const override;
        [[nodiscard]] FUUID
        GenerateUUID() const override;
        void
        SetCurrentThreadName(FStringView I_Name) const override;
        [[nodiscard]] IPlatformFileSystem*
        GetFileSystem() const override { return &FileSystem; }
        void
        PollEvents() const override { GLFW.PollEvents(); }
        void
        WaitEvents() const override { GLFW.WaitEvents(); }
        [[nodiscard]] IPlatformWindow*
        GetFocusedWindow() const override { return GLFW.GetFocusedWindow(); }
        [[nodiscard]] FStringView
        GetPlatformName() const override { return "MacOS"; }

    public:
        FMacOSPlatform();
        ~FMacOSPlatform() override = default;

    private:
        static std::string MakePlatformString(FStringView I_Text);

        mutable FMacOSPlatformFileSystem FileSystem;
        mutable FGLFWPlatform              GLFW;
    };

#ifndef VISERA_APP_NAME
#define VISERA_APP_NAME "Visera"
#endif

    FMacOSPlatform::FMacOSPlatform()
    {
        /* Configure log file sink per platform convention: ~/Library/Logs/AppName. */
        const FPath LogsDirectory = GetLogsDirectory();
        if (!LogsDirectory.IsEmpty())
        {
            (void)GetFileSystem()->CreateDirectories(FMacOSPath(LogsDirectory));
            FLog::SetSinkPath(LogsDirectory);
        }
    }

    FPath FMacOSPlatform::GetLogsDirectory() const
    {
        const TOptional<FString> Home = GetEnvironmentVariable("HOME");
        if (!Home.HasValue() || Home.GetValue().IsEmpty()) { return FPath(); }
        const FPath Base(Home.GetValue());
        return Base / FPath("Library") / FPath("Logs") / FPath(VISERA_APP_NAME);
    }

    TUniquePtr<IPlatformPath> FMacOSPlatform::GetExecutableDirectory() const
    {
        char   Path[PATH_MAX];
        UInt32 PathLength = sizeof(Path);
        if (_NSGetExecutablePath(Path, &PathLength) != 0)
        { LOG_FATAL("Failed to get executable path!"); }
        if (auto Parent = FPath(FString(Path)).GetParent(); Parent.HasValue())
        { return MakeUnique<FMacOSPath>(Parent.GetValue().GetString().GetNative()); }
        return nullptr;
    }

    TUniquePtr<IPlatformPath> FMacOSPlatform::GetResourceDirectory() const
    {
        if (auto Exec = GetExecutableDirectory(); Exec)
        {
            const FPath AppBundleDir = Exec->ToPath().GetParent().GetValue();
            return MakeUnique<FMacOSPath>((AppBundleDir / FPath{"Resources"}).GetString().GetNative());
        }
        return nullptr;
    }

    TUniquePtr<IPlatformPath> FMacOSPlatform::GetFrameworkDirectory() const
    {
        if (TUniquePtr<IPlatformPath> Exec = GetExecutableDirectory(); Exec)
        {
            const FPath AppBundleDir = Exec->ToPath().GetParent().GetValue();
            return MakeUnique<FMacOSPath>((AppBundleDir / FPath{"Frameworks"}).GetString().GetNative());
        }
        return nullptr;
    }

    TUniquePtr<IPlatformPath> FMacOSPlatform::GetUserDataDirectory() const
    {
        const TOptional<FString> Home = GetEnvironmentVariable("HOME");
        if (!Home.HasValue() || Home.GetValue().IsEmpty()) { return nullptr; }
        const FPath Base(Home.GetValue());
        const FPath UserDataDirectory = Base / FPath("Library") / FPath("Application Support") / FPath(VISERA_APP_NAME);
        return MakeUnique<FMacOSPath>(UserDataDirectory.GetString().GetNative());
    }

    std::string FMacOSPlatform::MakePlatformString(FStringView I_Text)
    {
        if (I_Text.IsEmpty()) { return {}; }
        return std::string(I_Text.Data(), I_Text.GetSize());
    }

    TUniquePtr<IPlatformWindow> FMacOSPlatform::
    CreateWindow(FStringView I_Title, UInt32 I_Width, UInt32 I_Height) const
    {
        return MakeUnique<FMacOSWindow>(I_Title, I_Width, I_Height);
    }

    Bool FMacOSPlatform::
    SetEnvironmentVariable(FStringView I_Variable,
                           FStringView I_Value) const
    {
        const std::string Var = MakePlatformString(I_Variable);
        const std::string Val = MakePlatformString(I_Value);
        if (setenv(Var.c_str(), Val.c_str(), True) != 0)
        {
            LOG_ERROR("Failed to set environment variable {} as {}!",
                      I_Variable, I_Value);
            return False;
        }
        LOG_DEBUG("Set environment variable {} as {}.",
                  Var, Val);
        return True;
    }

    TOptional<FString> FMacOSPlatform::
    GetEnvironmentVariable(FStringView I_Variable) const
    {
        const std::string Var = MakePlatformString(I_Variable);
        if (Var.empty()) { return std::nullopt; }
        const char* Val = ::getenv(Var.c_str());
        if (!Val) { return std::nullopt; }
        return FString(Val);
    }

    /**
     * Generates a UUID using macOS OS API.
     *
     * Notes:
     * - uuid_generate_random() produces an RFC 4122 version-4 UUID (random-based). :contentReference[oaicite:0]{index=0}
     * - uuid_t is a 16-byte array; we treat it as the canonical 16-octet UUID sequence.
     * - No endianness normalization is needed here as long as you always format/serialize via FUUID::Data[16].
     */
    FUUID FMacOSPlatform::
    GenerateUUID() const
    {
        FUUID UUID;
        ::uuid_generate_random(UUID.Data); // v4 random UUID :contentReference[oaicite:1]{index=1}
        return UUID;
    }

    void FMacOSPlatform::SetCurrentThreadName(FStringView I_Name) const
    {
        if (I_Name.IsEmpty()) { return; }
        std::string Name = MakePlatformString(I_Name);
        if (Name.size() > 63) { Name.resize(63); }
        (void)pthread_setname_np(Name.c_str());
    }
}