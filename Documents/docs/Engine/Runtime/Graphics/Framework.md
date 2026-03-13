# Runtime.Graphics.Framework (Visera.Runtime.Graphics.Framework)

Graphics framework: initialization and high-level setup for rendering. Defines the task and context types used by the main Graphics thread and the Render API.

## Types

### FRenderArea

Render extent (width × height). Used by `FRenderTask` and the Render API.

| Member | Type   | Description |
|--------|--------|-------------|
| `Width`  | `UInt32` | Internal render width; **0** → use SwapChain size. |
| `Height` | `UInt32` | Internal render height; **0** → use SwapChain size. |

### FRenderTask

Per-frame work item sent from the main thread to the Graphics thread via `TSPSCChannel<FRenderTask>`.

| Member      | Type           | Description |
|-------------|----------------|-------------|
| `SwapChainID` | `FRHISwapChainID` | Target swap chain. **`kInvalidSwapChainID`** is also used as the **channel poison pill** (stop the Graphics thread). |
| `Data`        | `FRenderData`    | Scene render data. |
| `RenderView`  | `FRenderView`    | View/projection. |
| `RenderArea`  | `FRenderArea`    | Render extent (0,0 = use SwapChain size). |

### FRenderContext

Per-frame context passed to registered pass factories. Built on the Graphics thread from the current `FRenderTask` and RHI state.

| Member         | Type              | Description |
|----------------|-------------------|-------------|
| `RHI`          | `FRHI*`           | RHI instance. |
| `Data`         | `const FRenderData*`  | Scene data. |
| `RenderView`   | `const FRenderView*`   | View. |
| `SwapChainID`  | `FRHISwapChainID` | Current swap chain. |
| `BackBuffer`   | `FRHITextureID`   | Current back buffer. |
| `PipelineCache`| `FPipelineCache*` | Per–swap chain PSO cache. |
| `RenderWidth`  | `UInt32`          | Resolved width (from `RenderArea` or SwapChain). |
| `RenderHeight` | `UInt32`          | Resolved height. |

### ERenderPassPriority

Priority constants for `RegisterPass`. Lower values run first. Example order: Setup → Opaque → OpaqueSprites → FinalBlit.

## Render API (FGraphics)

- **`Render(FWindow* I_Window, const FScene& I_Scene, const FRenderArea& I_RenderArea = {})`**  
  Convenience: resolves swap chain from the window; `I_RenderArea` default `{}` means derive size from the window. Forwards to the unified overload.

- **`Render(FRHISwapChainID I_SwapChainID, const FScene& I_Scene, const FRenderArea& I_RenderArea)`**  
  Unified entry: enqueues one `FRenderTask` to the Graphics thread. Used for headless or after the window overload.

Both overloads require an explicit `FRenderArea` at the call site for the unified overload; the window overload allows default `{}` (window size).

## Channel and poison pill

- **Channel type**: `TSPSCChannel<FRenderTask>` (no `TOptional` wrapper).
- **Poison pill**: Main thread sends `FRenderTask{}` (default `SwapChainID == kInvalidSwapChainID`) to request the Graphics thread to stop. The Graphics thread checks `RenderTask.SwapChainID == kInvalidSwapChainID` and exits its loop.

## See also

- [Graphics](index.md) — parent module
- [RHI](../RHI/index.md) — RHI backend
- [Scene](Scene/index.md) — scene management
