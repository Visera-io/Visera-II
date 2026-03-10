# Runtime.UI (Visera.Runtime.UI)

**Runtime.UI** provides the UI subsystem: context management and ImGui integration. context holds UI state and lifecycle(e.g. fonts, styles); ImGui module connects Dear ImGui to engine window, input and RHI, for editor UI, debug overlay and tool UI. rendering is done via RHI drawing ImGui vertex/index buffers and textures. 

## Submodules
| Module | Description |
|------|------|
| [Context](Context.md) | UI context and state.  |
| [ImGUI](ImGUI.md) | Dear ImGui integration.  |

## See also
- [Runtime](../index.md) — Parent module
- [Window](../Window/index.md) — window and present
- [Input](../Input/index.md) — input for ImGui
- [Graphics](../Graphics/index.md) — render pipeline
