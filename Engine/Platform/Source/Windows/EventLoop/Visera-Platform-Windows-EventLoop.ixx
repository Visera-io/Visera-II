module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Windows.EventLoop;
#define VISERA_MODULE_NAME "Platform.Windows"
import Visera.Platform.GLFW.EventLoop;

export namespace Visera
{
    using FWindowsEventLoop = FGLFWEventLoop;
}
