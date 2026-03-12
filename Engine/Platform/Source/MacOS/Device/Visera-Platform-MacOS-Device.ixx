module;
#include <Visera-Platform.hpp>
export module Visera.Platform.MacOS.Device;
#define VISERA_MODULE_NAME "Platform.MacOS"
export import Visera.Platform.GLFW.Device;

export namespace Visera
{
    using EMacOSMouseButton   = EGLFWMouseButton;
    using EMacOSKeyboardKey   = EGLFWKeyboardKey;
    using EMacOSKeyboardAction = EGLFWKeyboardAction;

    inline constexpr Int32 MacOSMouseButtonLast  = GLFWMouseButtonLast;
    inline constexpr Int32 MacOSKeyboardKeyLast  = GLFWKeyboardKeyLast;
}
