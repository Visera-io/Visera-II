module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Platform;
#define VISERA_MODULE_NAME "Runtime.Platform"
export import Visera.Runtime.Platform.Interface;
#if defined(VISERA_ON_WINDOWS_SYSTEM)
import Visera.Runtime.Platform.Windows;
#elif defined(VISERA_ON_APPLE_SYSTEM)
import Visera.Runtime.Platform.MacOS;
#endif

namespace Visera
{
    export using EPlatform = EPlatform;

#if defined(VISERA_ON_WINDOWS_SYSTEM)
    export inline VISERA_RUNTIME_API TUniquePtr<IPlatform>
    GPlatform = MakeUnique<FWindowsPlatform>();
#elif defined(VISERA_ON_APPLE_SYSTEM)
    export inline VISERA_RUNTIME_API TUniquePtr<IPlatform>
    GPlatform = MakeUnique<FMacOSPlatform>();
#endif
}