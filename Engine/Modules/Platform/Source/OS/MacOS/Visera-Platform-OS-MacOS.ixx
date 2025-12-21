module;
#include <Visera-Platform.hpp>
#if defined(VISERA_ON_APPLE_SYSTEM)
#include <mach-o/dyld.h>
#include <uuid/uuid.h>
#endif
export module Visera.Platform.OS.MacOS;
#define VISERA_MODULE_NAME "Platform.OS"
import Visera.Platform.OS.Interface;
import Visera.Platform.OS.MacOS.Library;
import Visera.Runtime.Log;
import Visera.Core.OS.FileSystem;

namespace Visera
{
#if defined(VISERA_ON_APPLE_SYSTEM)
    export class VISERA_PLATFORM_API FMacOSPlatform : public IOS
    {
    public:
        [[nodiscard]] TSharedPtr<ILibrary>
        LoadLibrary(const FPath& I_Path) const override { return MakeShared<FMacOSLibrary>(I_Path); }
        [[nodiscard]] const FPath&
        GetExecutableDirectory() const override { return ExecutableDirectory; }
        [[nodiscard]] const FPath&
        GetResourceDirectory() const override { return ResourceDirectory; }
        [[nodiscard]] const FPath&
        GetFrameworkDirectory() const override { return FrameworkDirectory; }
        [[nodiscard]] const FPath&
        GetCacheDirectory() const override { return CacheDirectory; }
        [[nodiscard]] Bool
        SetEnvironmentVariable(FStringView I_Variable, FStringView I_Value) const override;
        [[nodiscard]] UInt128
        GenerateUUID() const override { uuid_t UUID; uuid_generate(UUID); return {UUID[0], UUID[8]}; }
    private:
        FPath ExecutableDirectory;
        FPath ResourceDirectory;
        FPath FrameworkDirectory;
        FPath CacheDirectory;

    public:
        FMacOSPlatform();
        ~FMacOSPlatform() override = default;
    };

    FMacOSPlatform::
    FMacOSPlatform() : IOS{EPlatform::MacOS}
    {
        char Path[PATH_MAX];
        uint32_t PathLength = sizeof(Path);
        if (_NSGetExecutablePath(Path, &PathLength) == 0/*Success(0)*/)
        {
            ExecutableDirectory = FPath{Path}.GetParent();
        }
        else { LOG_FATAL("Failed to get executable path!"); }

        ResourceDirectory  = ExecutableDirectory.GetParent() / FPath{"Resources"};
        FrameworkDirectory = ExecutableDirectory.GetParent() / FPath{"Frameworks"};

        CacheDirectory     = ResourceDirectory / FPath{"Cache"};
        if (!FFileSystem::Exists(CacheDirectory))
        { (void)FFileSystem::CreateDirectory(CacheDirectory); }
    }

    Bool FMacOSPlatform::
    SetEnvironmentVariable(FStringView I_Variable,
                           FStringView I_Value) const
    {
        if (setenv(I_Variable.data(), I_Value.data(), True) != 0)
        {
            LOG_ERROR("Failed to set environment variable \"{}\" as \"{}\"!",
                      I_Variable, I_Value);
            return False;
        }
        LOG_DEBUG("Set environment variable \"{}\" as \"{}\".",
                  I_Variable, I_Value);
        return True;
    }
#endif
}