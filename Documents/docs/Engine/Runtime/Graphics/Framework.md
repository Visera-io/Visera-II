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

### FRenderBatch

Single batch for draw calls: shared pipeline and descriptor set, plus per-instance viewports.

| Member       | Type                | Description |
|--------------|---------------------|-------------|
| `Pipeline`     | `FRHIRenderPassID`    | PSO (from pipeline cache). |
| `DescriptorSet` | `FRHIDescriptorSetID` | Material descriptor set. |
| `Viewports`     | `TArray<FRHIViewport>` | One viewport per instance (e.g. per renderable). |

### FRenderList

Sorted list of render batches per surface type. Filled by `ExtractAndSortDrawList` on the Graphics thread from `FRenderData`; draw passes iterate only this list (no raw scene data).

| Member | Type | Description |
|--------|------|-------------|
| `OpaqueBatches`     | `TArray<FRenderBatch>` | Opaque batches. |
| `TransparentBatches`| `TArray<FRenderBatch>` | Transparent batches. |
| `WireframeBatches`  | `TArray<FRenderBatch>` | Cleared but not filled by extraction. |

### FRenderContext

Per-frame context passed to registered pass factories. **Draw passes** use `RenderList` only (batched by material/PSO). **Setup passes** use `RenderWidth` / `RenderHeight`. Built on the Graphics thread after extracting the draw list from `FRenderTask.Data`.

| Member         | Type                | Description |
|----------------|---------------------|-------------|
| `RenderList`   | `const FRenderList*` | Sorted batches (opaque/transparent); required for draw passes. |
| `RHI`          | `FRHI*`             | RHI instance. |
| `SwapChainID`  | `FRHISwapChainID`   | Current swap chain. |
| `BackBuffer`   | `FRHITextureID`     | Current back buffer. |
| `PipelineCache`| `FPipelineCache*`   | Per–swap chain PSO cache. |
| `RenderWidth`  | `UInt32`            | Resolved width (from `RenderArea` or SwapChain). |
| `RenderHeight` | `UInt32`            | Resolved height. |

### ERenderPassPriority

Priority constants for `RegisterPass`. Lower values run first. Example order: Setup → Opaque → OpaqueSprites → FinalBlit.

## Render API (FGraphics)

- **`Render(FWindow* I_Window, const FScene& I_Scene, const FRenderArea& I_RenderArea = {})`**  
  Convenience: resolves swap chain from the window; `I_RenderArea` default `{}` means derive size from the window. Forwards to the unified overload.

- **`Render(FRHISwapChainID I_SwapChainID, const FScene& I_Scene, const FRenderArea& I_RenderArea)`**  
  Unified entry: enqueues one `FRenderTask` to the Graphics thread. Used for headless or after the window overload.

Both overloads require an explicit `FRenderArea` at the call site for the unified overload; the window overload allows default `{}` (window size).

## Draw list extraction (main module)

**`ExtractAndSortDrawList(I_Data, O_List, I_PipelineCache, I_RHI, I_ColorFormats, I_DepthFormat)`** (in `Visera.Runtime.Graphics`) fills `O_List` by iterating `I_Data` renderables, batching by (Material, PSO). Routes by surface: opaque vs transparent. `WireframeBatches` is cleared but not filled. The Graphics thread calls this once per frame before building `FRenderContext` and running passes.

## RegisterPass and concurrency

**`RegisterPass`** is write-locked (`FRWLock`); the Graphics thread snapshots the pass array under a read lock each frame so that the main thread can register passes concurrently without iterating a changing list.

## Channel and poison pill

- **Channel type**: `TSPSCChannel<FRenderTask>` (no `TOptional` wrapper).
- **Poison pill**: Main thread sends `FRenderTask{}` (default `SwapChainID == kInvalidSwapChainID`) to request the Graphics thread to stop. The Graphics thread checks `RenderTask.SwapChainID == kInvalidSwapChainID` and exits its loop.

## See also

- [Graphics](index.md) — parent module
- [RHI](../RHI/index.md) — RHI backend
- [Scene](Scene/index.md) — scene management
