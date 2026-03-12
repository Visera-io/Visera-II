module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Windows.Device;
#define VISERA_MODULE_NAME "Platform.Windows"
export import Visera.Platform.GLFW.Device;

export namespace Visera
{
    using EWindowsMouseButton   = EGLFWMouseButton;
    using EWindowsKeyboardKey   = EGLFWKeyboardKey;
    using EWindowsKeyboardAction = EGLFWKeyboardAction;

    inline constexpr Int32 WindowsMouseButtonLast  = GLFWMouseButtonLast;
    inline constexpr Int32 WindowsKeyboardKeyLast = GLFWKeyboardKeyLast;
}
