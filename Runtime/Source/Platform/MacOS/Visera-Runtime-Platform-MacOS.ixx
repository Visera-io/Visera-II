module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Platform.MacOS;
#define VISERA_MODULE_NAME "Runtime.Platform"
import Visera.Runtime.Platform.Interface;
import Visera.Runtime.Platform.MacOS.Library;
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_RUNTIME_API FMacOSPlatform : public IPlatform
    {
    public:
        [[nodiscard]] TSharedPtr<ILibrary>
        LoadLibrary(const FPath& I_Path) const override;

    public:
        FMacOSPlatform() : IPlatform{EPlatform::MacOS} {};
        ~FMacOSPlatform() override = default;
    };

    TSharedPtr<ILibrary> FMacOSPlatform::
    LoadLibrary(const FPath& I_Path) const
    {
        LOG_DEBUG("Loading library {}", I_Path);
        return MakeShared<FMacOSLibrary>(I_Path);
    }
}