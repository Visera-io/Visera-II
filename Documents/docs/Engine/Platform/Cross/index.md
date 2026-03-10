# Platform.Cross (Visera.Platform.Cross)

**Platform.Cross** provides cross-platform or platform-agnostic implementation: **GLFW** and **Null**. GLFW provides window and input (keyboard, mouse) via GLFW on Windows, macOS, Linux; Null provides no-window, no-input placeholder for headless, unit test or server builds. Aligned with [Interface](../Interface/index.md) so switching between windowed and headless builds is easy.

## Submodules
| Module | Description |
|------|------|
| [GLFW](GLFW/index.md) | GLFW window, [Keyboard](GLFW/Keyboard.md), [Mouse](GLFW/Mouse.md). |
| [Null](Null/index.md) | Placeholder [Window](Null/Window.md). |

## See also
- [Platform](../index.md) — Parent module
- [Interface](../Interface/index.md) — Abstract API
- [Runtime.Input](../../Runtime/Input/index.md) — Runtime input consumes GLFW events
