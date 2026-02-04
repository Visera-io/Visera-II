module;
#include <Visera-Platform.hpp>
export module Visera.Platform.OS;
#define VISERA_MODULE_NAME "Platform.OS"
#if defined(VISERA_ON_WINDOWS_SYSTEM)
import Visera.Platform.OS.Windows;
#elif defined(VISERA_ON_APPLE_SYSTEM)
import Visera.Platform.OS.MacOS;
#endif
import Visera.Platform.OS.Interface;
import Visera.Core.OS.FileSystem;
import Visera.Global;

namespace Visera
{
    export class VISERA_PLATFORM_API FPlatform : public IGlobalService
    {
    public:
        [[nodiscard]] inline TSharedPtr<ILibrary>
        LoadLibrary(const FPath& I_Path) const { return Platform->LoadLibrary(I_Path); }
        [[nodiscard]] inline const FPath&
        GetExecutableDirectory() const { return Platform->GetExecutableDirectory(); }
        [[nodiscard]] inline const FPath&
        GetResourceDirectory() const { return Platform->GetResourceDirectory(); }
        [[nodiscard]] inline const FPath&
        GetFrameworkDirectory() const { return Platform->GetFrameworkDirectory(); }
        [[nodiscard]] inline Bool
        SetEnvironmentVariable(FStringView I_Variable, FStringView I_Value) const { return Platform->SetEnvironmentVariable(I_Variable, I_Value); }
        [[nodiscard]] inline FUUID
        GenerateUUID() const { return Platform->GenerateUUID(); }

        [[nodiscard]] inline EPlatform
        GetType() const { return Platform->GetType(); }

    private:
        IOS* Platform {nullptr};

    public:
        FPlatform() : IGlobalService(EName::Platform)
        {
            Dependencies =
            {

            };

            if (!OnBootstrap.TryBind([this]
            {
#if defined(VISERA_ON_WINDOWS_SYSTEM)
                Platform = new FWindowsPlatform();
#elif defined(VISERA_ON_APPLE_SYSTEM)
                Platform = new FMacOSPlatform();
#endif
                if (!Platform) { return False; }

                if (auto Error = FFileSystem::CreateDirectory(Platform->GetCacheDirectory()); Error)
                {
                    LOG_ERROR("Failed to create cache directory -- {}!", Error.message());
                    return False;
                }

                return True;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                delete Platform;
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        }
    };
}