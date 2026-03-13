# Platform.GLFW.Window (Visera.Platform.GLFW.Window)

GLFW window implementation. Creates and manages a GLFW window.

## Focus

The static focused-window pointer used for input is set in the window-focus callback. So that the newly created window is usable for input before the OS sends the first focus event, the constructor sets the new window as the current focused window (`FocusedWindow = this`) after registering the callback; the callback will update it when focus is gained or lost.

## Content scale and cursor

On high-DPI displays (e.g. macOS Retina), the window has a content scale (`GetScaleX()` / `GetScaleY()`). The cursor-position callback multiplies the reported position by this scale before invoking `CursorMoveCallback`, so Runtime.Input receives coordinates in framebuffer (pixel) space. When the scale changes (e.g. window moved to another monitor), `glfwSetWindowContentScaleCallback` updates the window’s scale via `SetContentScale` and then invokes `WindowContentScaleCallback`.

## See also

- [GLFW](index.md) — parent module
- [Interface.Window](../Interface/Window.md) — window abstraction
