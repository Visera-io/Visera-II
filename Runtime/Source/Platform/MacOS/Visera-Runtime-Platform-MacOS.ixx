module;
#include <Visera-Runtime.hpp>
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

    public:
        FMacOSPlatform() : IPlatform{EPlatform::MacOS} {};
        ~FMacOSPlatform() override = default;
    };
#endif
}