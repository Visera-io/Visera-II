module;
#include <Visera-Platform.hpp>
#include <GLFW/glfw3.h>
export module Visera.Platform.GLFW.Device.Mouse;
#define VISERA_MODULE_NAME "Platform.GLFW"
import Visera.Platform.Interface.Device.Mouse;

export namespace Visera
{
    /** Alias for Interface.Device type; GLFW uses same values. static_assert keeps them in sync. */
    using EGLFWMouseButton = EPlatformMouseButton;
    static_assert(static_cast<Int32>(EPlatformMouseButton::Left) == GLFW_MOUSE_BUTTON_LEFT);
    static_assert(static_cast<Int32>(EPlatformMouseButton::Button8) == GLFW_MOUSE_BUTTON_8);

    inline constexpr Int32 GLFWMouseButtonLast = GLFW_MOUSE_BUTTON_LAST;
}
