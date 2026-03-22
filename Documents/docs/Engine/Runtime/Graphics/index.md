# Runtime.Graphics (Visera.Runtime.Graphics)

**Runtime.Graphics** builds high-level graphics on top of RHI: framework init and lifecycle, material system, pipeline cache, render graph and scene (camera, light, renderable). Uses an **immediate-mode draw API**: the game submits per-frame via `SetCamera`, `SubmitLight` and `Draw`; the engine calls `Render(FWindow*)` after `OnPreRender` to send accumulated data to the Graphics thread. Converts draw commands and materials to RHI commands and resource bindings; manages multi-pass and resource dependencies via render graph.

## Responsibilities
- **Framework**: Graphics subsystem init; `FRenderTask` / `FRenderContext`; **draw list** from `FRenderData::DrawCommands` (`BatchAndSort` → `FRenderList` by material/mesh); **instance buffer upload** (`UploadInstanceBuffers` → per-batch SSBO + descriptor set); pass registration with read/write lock for concurrency.
- **Material**: Material type, render state (cull mode, depth test/write/compare), descriptor set 0 (textures/samplers); JSON `.vmat` driven.
- **PipelineCache**: Get-or-create PSO by material shaders, formats and render state; **hash-based O(1)** lookup (GoldenRatio); key includes depth/stencil and cull configuration.
- **RenderGraph**: Declarative passes; **Execute(FRenderContext)** (creates command list, runs passes with `RenderList`, submits); pass names as `FName`; **CullDeadPasses** O(N+E) reverse BFS.
- **Scene**: Camera (`FCamera`), lights (`FLight`), renderables (`IRenderable` / `FRenderableMeta` → `FInstanceData` + Material + optional `FMesh`). All submitted per-frame via `FGraphics`; no persistent scene container (`FScene` removed).

## GPU instancing pipeline

1. Game calls `GFX->Draw(renderable)` or `GFX->Draw(FRenderableMeta)`; data is captured at submit time.
2. After `OnPreRender`, engine calls `GFX->Render(Window)` to build `FRenderTask` from accumulated draws, camera and lights.
3. Graphics thread runs `BatchAndSort` on `FRenderData::DrawCommands` → `FRenderList` (batches by Material + Mesh).
4. `UploadInstanceBuffers` creates per-batch SSBO and descriptor set.
5. Draw passes bind set 0 (material) + set 1 (instances) and issue `Draw(6, N)` or `DrawIndexed(IndexCount, N)`.

## Submodules
| Module | Description |
|------|------|
| [Framework](Framework.md) | Graphics framework init, FRenderBatch, FRenderList, instance upload. |
| [Material](Material.md) | Material type, render state and descriptor set 0. |
| [PipelineCache](PipelineCache.md) | Pipeline cache with extended key. |
| [RenderGraph](RenderGraph.md) | Render graph. |
| [Scene](Scene/index.md) | [Camera](Scene/Camera.md), [Light](Scene/Light.md), [Renderable](Scene/Renderable.md). |

## See also
- [Runtime](../index.md) — Parent module
- [RHI](../RHI/index.md) — Low-level render interface
- [AssetHub](../AssetHub/index.md) — Texture and shader asset loading
- [Transform](../../Core/Math/Geometry/Transform.md) — FTransform3x4F
