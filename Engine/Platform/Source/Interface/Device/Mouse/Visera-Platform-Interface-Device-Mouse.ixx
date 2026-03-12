module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Interface.Device.Mouse;
#define VISERA_MODULE_NAME "Platform.Interface"

// Platform-agnostic mouse button and button state. Values aligned with GLFW_MOUSE_BUTTON_*
// and GLFW_RELEASE/GLFW_PRESS so Platform implementations can static_cast at the boundary.
// No platform header is included; platform differences stay under the Platform layer.

export namespace Visera
{
    /** Mouse button identifier. Values match GLFW_MOUSE_BUTTON_*. */
    enum class EPlatformMouseButton : Int32
    {
        Left   = 0,
        Right  = 1,
        Middle = 2,
        Button4 = 3,
        Button5 = 4,
        Button6 = 5,
        Button7 = 6,
        Button8 = 7,
    };

    /** Button state from query or callback. Values match GLFW_RELEASE/GLFW_PRESS. */
    enum class EPlatformMouseButtonState : Int32
    {
        Release = 0,
        Press   = 1,
    };

    /** Table size for QueryMouseButtonState(O_Out): buttons 0..7 (Left..Button8). */
    inline constexpr size_t kMouseButtonStateTableSize = 8;
}
