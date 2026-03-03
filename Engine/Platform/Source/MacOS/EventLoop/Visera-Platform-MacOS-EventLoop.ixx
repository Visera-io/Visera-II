module;
#include <Visera-Platform.hpp>
export module Visera.Platform.MacOS.EventLoop;
#define VISERA_MODULE_NAME "Platform.MacOS"
import Visera.Platform.Cross.GLFW.EventLoop;

export namespace Visera
{
    using FMacOSEventLoop = FGLFWEventLoop;
}
