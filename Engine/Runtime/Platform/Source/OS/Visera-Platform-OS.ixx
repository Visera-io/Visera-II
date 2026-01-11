module;
#include <Visera-Platform.hpp>
export module Visera.Platform.OS;
#define VISERA_MODULE_NAME "Platform.OS"
#if defined(VISERA_ON_WINDOWS_SYSTEM)
export import Visera.Platform.OS.Windows;
#elif defined(VISERA_ON_APPLE_SYSTEM)
export import Visera.Platform.OS.MacOS;
#endif

namespace Visera
{
    
}