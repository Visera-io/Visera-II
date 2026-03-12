module;
#include <Visera-Platform.hpp>
#include <GLFW/glfw3.h>
export module Visera.Platform.GLFW.Device.Mouse;
#define VISERA_MODULE_NAME "Platform.GLFW"

export namespace Visera
{
    enum class EGLFWMouseButton : Int32
    {
        Left   = GLFW_MOUSE_BUTTON_LEFT,
        Right  = GLFW_MOUSE_BUTTON_RIGHT,
        Middle = GLFW_MOUSE_BUTTON_MIDDLE,
        Button4 = GLFW_MOUSE_BUTTON_4,
        Button5 = GLFW_MOUSE_BUTTON_5,
        Button6 = GLFW_MOUSE_BUTTON_6,
        Button7 = GLFW_MOUSE_BUTTON_7,
        Button8 = GLFW_MOUSE_BUTTON_8,
    };

    inline constexpr Int32 GLFWMouseButtonLast = GLFW_MOUSE_BUTTON_LAST;
}
