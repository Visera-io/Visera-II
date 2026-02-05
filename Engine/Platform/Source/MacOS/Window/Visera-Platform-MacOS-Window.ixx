module;
#include <Visera-Platform.hpp>
export module Visera.Platform.MacOS.Window;
#define VISERA_MODULE_NAME "Platform.MacOS"
import Visera.Platform.Cross.Window;

export namespace Visera
{
    using FMacOSWindow = FGLFWWindow;
}