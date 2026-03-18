# Runtime.Graphics.Framework (Visera.Runtime.Graphics.Framework)

Graphics framework: initialization and high-level setup for rendering. Defines the task and context types used by the main Graphics thread and the Render API.

## Types

### FRenderArea

Render extent (width × height). Used by `FRenderTask` and the Render API.

| Member | Type   | Description |
|--------|--------|-------------|
| `Width`  | `UInt32` | Internal render width; **0** → use SwapChain size. |
| `Height` | `UInt32` | Internal render height; **0** → use SwapChain size. |

### FRenderData

Per-frame payload of draw commands and lights. Built by `FGraphics::Render(FWindow*)` from accumulated `DrawCommands` and `FrameLights`; read-only on the Graphics thread.

| Member        | Type                      | Description |
|---------------|---------------------------|-------------|
| `DrawCommands`| `TArray<FRenderableMeta>` | Draw commands (instance data + material + mesh); filled from `Draw()` calls. |
| `Lights`      | `TArray<FLight>`          | Lights for the frame. |

### FRenderableMeta

Extracted draw-command data (no virtual calls across threads). Used by `Draw(const FRenderableMeta&)` and produced by `IRenderable::ToRenderableMeta()`.

| Member        | Type                   | Description |
|---------------|------------------------|-------------|
| `InstanceData`| `FInstanceData`        | Transform, color, custom data. |
| `Material`    | `TSharedPtr<FMaterial>` | Material; required. |
| `Mesh`        | `TSharedPtr<FMesh>`     | Mesh geometry; `nullptr` = sprite quad path. |

### FRenderTask

Per-frame work item sent from the main thread to the Graphics thread via `TSPSCChannel<FRenderTask>`.

| Member      | Type           | Description |
|-------------|----------------|-------------|
| `SwapChainID` | `FRHISwapChainID` | Target swap chain. **`kInvalidSwapChainID`** is also used as the **channel poison pill** (stop the Graphics thread). |
| `Data`        | `FRenderData`    | Draw commands and lights. |
| `RenderView`  | `FRenderView`    | View/projection (from `SetCamera`). |
| `RenderArea`  | `FRenderArea`    | Render extent (0,0 = use SwapChain size). |

### FRenderBatch

Single batch for instanced draw calls. All instances in a batch share the same pipeline, material and mesh.

| Member                 | Type                      | Description |
|------------------------|---------------------------|-------------|
| `Pipeline`             | `FRHIRenderPassID`        | PSO (from pipeline cache). |
| `MaterialDescriptorSet`| `FRHIDescriptorSetID`     | Descriptor set 0: material textures/samplers. |
| `Mesh`                 | `TSharedPtr<FMesh>`       | Mesh geometry; `nullptr` → built-in sprite quad path. |
| `Instances`            | `TArray<FInstanceData>`   | CPU-side per-instance data (transform, color, custom). |
| `InstanceBuffer`       | `FRHIBufferID`            | GPU storage buffer holding `FInstanceData[]`. Populated by `UploadInstanceBuffers`. |
| `InstanceDescriptorSet`| `FRHIDescriptorSetID`     | Descriptor set 1: binds `InstanceBuffer` at binding 0. Populated by `UploadInstanceBuffers`. |

**Batch key**: `Pipeline + Material + Mesh`. Renderables sharing the same material and mesh are merged into one batch; a single `Draw(6, N)` or `DrawIndexed(IndexCount, N)` call renders all instances.

### FRenderList

Sorted list of render batches per surface type. Filled by `BatchAndSort` on the Graphics thread from `FRenderData::DrawCommands`; draw passes iterate only this list.

| Member | Type | Description |
|--------|------|-------------|
| `OpaqueBatches`     | `TArray<FRenderBatch>` | Opaque batches. |
| `TransparentBatches`| `TArray<FRenderBatch>` | Transparent batches. |
| `WireframeBatches`  | `TArray<FRenderBatch>` | Cleared but not filled by extraction. |

### FRenderContext

Per-frame context passed to registered pass factories. **Draw passes** use `RenderList` only (batched by material/PSO/mesh). **Setup passes** use `RenderWidth` / `RenderHeight`. Built on the Graphics thread after extracting and uploading the draw list from `FRenderTask.Data`.

| Member         | Type                | Description |
|----------------|---------------------|-------------|
| `RenderList`   | `const FRenderList*` | Sorted batches (opaque/transparent); required for draw passes. |
| `RenderView`   | `const FRenderView*` | View/projection data for the current frame. |
| `RHI`          | `FRHI*`             | RHI instance. |
| `SwapChainID`  | `FRHISwapChainID`   | Current swap chain. |
| `BackBuffer`   | `FRHITextureID`     | Current back buffer. |
| `PipelineCache`| `FPipelineCache*`   | Per–swap chain PSO cache. |
| `RenderWidth`  | `UInt32`            | Resolved width (from `RenderArea` or SwapChain). |
| `RenderHeight` | `UInt32`            | Resolved height. |

### ERenderPassPriority

Priority constants for `RegisterPass`. Lower values run first. Example order: Setup → Opaque → OpaqueSprites → FinalBlit.

## Render API (FGraphics)

All of the following are **per-frame**; state is cleared after `Render(FWindow*)` is called (by the engine after `OnPreRender`).

- **`SetCamera(FCamera I_Camera)`** — Set view/projection for this frame. Cleared after `Render`.
- **`SubmitLight(FLight I_Light)`** — Add a light for this frame. Cleared after `Render`.
- **`Draw(const IRenderable& I_Renderable)`** — Submit a draw; data is extracted via `ToRenderableMeta()` and stored.
- **`Draw(const FRenderableMeta& I_Meta)`** — Submit a draw from pre-built meta (e.g. scripting bindings).
- **`Render(FWindow* I_Window)`** — Build `FRenderTask` from accumulated draws, camera and lights; send to the Graphics thread; clear per-frame state. **Called by the engine** after `OnPreRender` (single-window case). Resolves swap chain from the window and derives `FRenderArea` from window size.

## Draw list extraction (main module)

**`BatchAndSort(I_Data, O_List)`** (in `Visera.Runtime.Graphics`) fills `O_List` by iterating `I_Data.GetDrawCommands()` (each element is `FRenderableMeta`), batching by **(Material, Mesh)**. The batch key is computed from `Material*` and `Mesh*` via `Math::GoldenRatioHashCombine`. Routes by surface: opaque vs transparent. `WireframeBatches` is cleared but not filled. The Graphics thread calls this once per frame before uploading instance buffers and building `FRenderContext`.

## Instance buffer upload

**`UploadInstanceBuffers(IO_List, I_RHI)`** — called once per frame after `BatchAndSort` and before `FRenderContext` construction. For each batch with instances, creates a storage buffer (`FInstanceData[]`), uploads it, creates a descriptor set (set 1, binding 0 = StorageBuffer) and writes the buffer binding. Buffer lifetime is tied to `FRHIBufferID` RAII — destroyed when `FRenderList` goes out of scope at end of frame.

## RegisterPass and concurrency

**`RegisterPass`** is write-locked (`FRWLock`); the Graphics thread snapshots the pass array under a read lock each frame so that the main thread can register passes concurrently without iterating a changing list.

## Channel and poison pill

- **Channel type**: `TSPSCChannel<FRenderTask>` (no `TOptional` wrapper).
- **Poison pill**: Main thread sends `FRenderTask{}` (default `SwapChainID == kInvalidSwapChainID`) to request the Graphics thread to stop. The Graphics thread checks `RenderTask.SwapChainID == kInvalidSwapChainID` and exits its loop.

## See also

- [Graphics](index.md) — parent module
- [RHI](../RHI/index.md) — RHI backend
- [Scene](Scene/index.md) — scene management
- [Renderable](Scene/Renderable.md) — FInstanceData and FMesh
