module;
#include <Visera-Runtime.hpp>
#if defined(VISERA_ON_APPLE_SYSTEM)
#include <mach-o/dyld.h>
#endif
export module Visera.Runtime.Platform.MacOS;
#define VISERA_MODULE_NAME "Runtime.Platform"
import Visera.Runtime.Platform.Interface;
import Visera.Runtime.Platform.MacOS.Library;
import Visera.Core.Log;

namespace Visera
{
#if defined(VISERA_ON_APPLE_SYSTEM)
    export class VISERA_RUNTIME_API FMacOSPlatform : public IPlatform
    {
    public:
        [[nodiscard]] TSharedPtr<ILibrary>
        LoadLibrary(const FPath& I_Path) const override { return MakeShared<FMacOSLibrary>(I_Path); }
        [[nodiscard]] const FPath&
        GetExecutableDirectory() const override { return ExecutableDirectory; }

    private:
        FPath ExecutableDirectory;

    public:
        FMacOSPlatform();
        ~FMacOSPlatform() override = default;
    };

    FMacOSPlatform::
    FMacOSPlatform() : IPlatform{EPlatform::MacOS}
    {
        char Path[PATH_MAX];
        uint32_t PathLength = sizeof(Path);
        if (_NSGetExecutablePath(Path, &PathLength) == 0/*Success(0)*/)
        {
            ExecutableDirectory = FPath{reinterpret_cast<char8_t*>(Path)}.GetParent();
        }
        else { LOG_FATAL("Failed to get executable path!"); }
    }
#endif
}