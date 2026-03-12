# Platform.GLFW.Window (Visera.Platform.GLFW.Window)

GLFW window implementation. Creates and manages a GLFW window.

## Focus

The static focused-window pointer used for input is set in the window-focus callback. So that the newly created window is usable for input before the OS sends the first focus event, the constructor sets the new window as the current focused window (`FocusedWindow = this`) after registering the callback; the callback will update it when focus is gained or lost.

## See also

- [GLFW](index.md) — parent module
- [Interface.Window](../Interface/Window.md) — window abstraction
