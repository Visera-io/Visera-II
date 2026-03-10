# Runtime.Window (Visera.Runtime.Window)

**Runtime.Window** binds the [Platform](../../Platform/index.md) window abstraction to Runtime rendering and input: creates the RHI swap chain on the platform window, handles resize and present, and may pass window handle or events to Input and UI. Applications or editors typically create the window via Platform first, then bind the swap chain and frame loop through this module.

## Responsibilities

- Hold or reference the Platform window and RHI swap chain; coordinate per-frame Present and swap chain recreation on resize.
- Notify other parts of the engine of window events (e.g. close, resize); Input may receive keyboard and mouse events from the same window (via Platform or GLFW).

## See also

- [Runtime](../index.md) — Parent module
- [Platform](../../Platform/index.md) — Platform window abstraction
- [RHI.SwapChain](../RHI/SwapChain.md) — Swap chain and present
