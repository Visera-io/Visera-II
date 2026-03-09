module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Windows.Window;
#define VISERA_MODULE_NAME "Platform.Windows"
export import Visera.Platform.Cross.GLFW.Window;

export namespace Visera
{
    using FWindowsWindow = FGLFWWindow;
}