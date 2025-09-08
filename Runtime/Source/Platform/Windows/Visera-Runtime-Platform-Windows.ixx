module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Platform.Windows;
#define VISERA_MODULE_NAME "Runtime.Platform"
import Visera.Runtime.Platform.Interface;
import Visera.Runtime.Platform.Windows.Library;
import Visera.Core.Log;

namespace Visera
{
#if defined(VISERA_ON_WINDOWS_SYSTEM)
    export class VISERA_RUNTIME_API FWindowsPlatform : public IPlatform
    {
    public:
        [[nodiscard]] TSharedPtr<ILibrary>
        LoadLibrary(const FPath& I_Path) const override { return MakeShared<FWindowsLibrary>(I_Path); }

    public:
        FWindowsPlatform() : IPlatform{EPlatform::Windows} {};
        ~FWindowsPlatform() override = default;
    };
#endif
}