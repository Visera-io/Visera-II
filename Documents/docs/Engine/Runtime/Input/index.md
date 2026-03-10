# Runtime.Input (Visera.Runtime.Input)

**Runtime.Input** provides input subsystem: maps raw device input (keyboard, mouse) to logical actions and axes for game and UI. Supports multiple devices, action press/release/hold state, and mapping config to bind keys/buttons to named actions for cross-platform, configurable input.

## Responsibilities
- **Action**: Define logical actions (e.g. Jump, MoveX); boolean (press/release) and scalar (axis); game code depends on action name, not physical key.
- **Device**: Abstract input device; keyboard and mouse key/button and motion state.
- **Mapping**: Map physical input (key code, button, axis) to actions; multiple mapping sets or runtime switch.

Relation to [Platform.Cross.GLFW](../../Platform/Cross/GLFW/index.md): GLFW provides low-level window and input events; Runtime.Input consumes them and turns them into actions and device state for upper layers.

## Submodules
| Module | Description |
|------|------|
| [Action](Action.md) | Input action type and state. |
| [Device](Device/index.md) | [Keyboard](Device/Keyboard.md), [Mouse](Device/Mouse.md). |
| [Mapping](Mapping.md) | Input mapping config and parsing. |

## See also
- [Runtime](../index.md) — Parent module
- [Platform.Cross.GLFW](../../Platform/Cross/GLFW/index.md) — GLFW keyboard and mouse
- [UI](../UI/index.md) — UI may consume input
