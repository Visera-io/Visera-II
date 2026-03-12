module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Windows.Window;
#define VISERA_MODULE_NAME "Platform.Windows"
/** Re-export GLFW window implementation; Windows platform uses it via FGLFWPlatform composition. */
export import Visera.Platform.GLFW.Window;

export namespace Visera
{
    using FWindowsWindow = FGLFWWindow;
}