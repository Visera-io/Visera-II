module;
#include <Visera-Platform.hpp>
#if defined(VISERA_ON_APPLE_SYSTEM)
#include <mach-o/dyld.h>
#include <uuid/uuid.h>
#endif
export module Visera.Platform.MacOS;
#define VISERA_MODULE_NAME "Platform.MacOS"
import Visera.Platform.Interface;
import Visera.Platform.MacOS.Window;
import Visera.Platform.MacOS.Library;
import Visera.Core.Types.Path;
import Visera.Core.Types.String;
import Visera.Core.OS.FileSystem;
import Visera.Global.Log;

export namespace Visera
{
#if defined(VISERA_ON_APPLE_SYSTEM)
    class VISERA_PLATFORM_API FMacOSPath : public IPlatformPath
    {
    public:
        explicit FMacOSPath(const FPath& I_Path) : Native(I_Path.GetString().GetNative())
        {
            VISERA_ASSERT(I_Path.IsNormalized());
        }
        explicit FMacOSPath(std::string_view I_Native) : Native(I_Native) {}
        [[nodiscard]] operator std::string_view() const noexcept { return Native; }
        [[nodiscard]] FPath ToPath() const override { return FPath(FString(*this)); }

    private:
        std::string Native;
    };

    class VISERA_PLATFORM_API FMacOSPlatform : public IPlatform
    {
    public:
        [[nodiscard]] TUniquePtr<IPlatformWindow>
        CreateWindow(FStringView I_Title, UInt32 I_Width, UInt32 I_Height) const override;
        [[nodiscard]] TSharedPtr<IPlatformLibrary>
        LoadLibrary(const IPlatformPath& I_Path) const override { return MakeShared<FMacOSLibrary>(I_Path.ToPath()); }
        [[nodiscard]] TUniquePtr<IPlatformPath>
        GetExecutableDirectory() const override;
        [[nodiscard]] TUniquePtr<IPlatformPath>
        GetResourceDirectory() const override;
        [[nodiscard]] TUniquePtr<IPlatformPath>
        GetFrameworkDirectory() const override;
        [[nodiscard]] Bool
        SetEnvironmentVariable(FStringView I_Variable, FStringView I_Value) const override;
        [[nodiscard]] FUUID
        GenerateUUID() const override;

    public:
        FMacOSPlatform();
        ~FMacOSPlatform() override = default;
    };

    FMacOSPlatform::FMacOSPlatform() : IPlatform{EPlatform::MacOS} {}

    TUniquePtr<IPlatformPath> FMacOSPlatform::GetExecutableDirectory() const
    {
        char Path[PATH_MAX];
        uint32_t PathLength = sizeof(Path);
        if (_NSGetExecutablePath(Path, &PathLength) != 0)
        { LOG_FATAL("Failed to get executable path!"); }
        const FPath ExePath(Path);
        if (auto Parent = ExePath.GetParent(); Parent.HasValue())
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

    TUniquePtr<IPlatformWindow> FMacOSPlatform::
    CreateWindow(FStringView I_Title, UInt32 I_Width, UInt32 I_Height) const
    {
        return MakeUnique<FMacOSWindow>(I_Title, I_Width, I_Height);
    }

    Bool FMacOSPlatform::
    SetEnvironmentVariable(FStringView I_Variable,
                           FStringView I_Value) const
    {
        if (setenv(I_Variable.Data(), I_Value.Data(), True) != 0)
        {
            LOG_ERROR("Failed to set environment variable {} as {}!",
                      I_Variable, I_Value);
            return False;
        }
        LOG_DEBUG("Set environment variable {} as {}.",
                  I_Variable, I_Value);
        return True;
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
#endif
}