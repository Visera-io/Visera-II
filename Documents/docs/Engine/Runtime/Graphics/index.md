# Runtime.Graphics (Visera.Runtime.Graphics)

**Runtime.Graphics** builds high-level graphics on top of RHI: framework init and lifecycle, material system, pipeline cache, render graph and scene (camera, light, renderable). Converts scene and materials to RHI commands and resource bindings; manages multi-pass and resource dependencies via render graph.

## Responsibilities
- **Framework**: Graphics subsystem init; `FRenderTask` / `FRenderContext`; **draw list** extraction (`ExtractAndSortDrawList` → `FRenderList` by material/mesh/PSO); **instance buffer upload** (`UploadInstanceBuffers` → per-batch SSBO + descriptor set 1); pass registration with read/write lock for concurrency.
- **Material**: Material type, render state (cull mode, depth test/write/compare), descriptor set 0 (textures/samplers); JSON `.vmaterial` driven.
- **PipelineCache**: Get-or-create PSO by material shaders, formats and render state; **hash-based O(1)** lookup (GoldenRatio); key includes depth/stencil and cull configuration.
- **RenderGraph**: Declarative passes; **Execute(FRenderContext)** (creates command list, runs passes with `RenderList`, submits); pass names as `FName`; **CullDeadPasses** O(N+E) reverse BFS.
- **Scene**: Camera (view/projection), lights, renderables (`IRenderable` → `FInstanceData` + optional `FMesh`); scene data is consumed once per frame to build `FRenderList`, then draw passes use only the list.

## GPU instancing pipeline

1. `IRenderable::GetInstanceData()` provides `FInstanceData` (transform, color, custom data).
2. `ExtractAndSortDrawList` groups by **Pipeline + Material + Mesh** into `FRenderBatch`.
3. `UploadInstanceBuffers` creates per-batch SSBO (`FInstanceData[]`) and descriptor set 1.
4. Draw passes bind set 0 (material) + set 1 (instances) and issue `Draw(6, N)` or `DrawIndexed(IndexCount, N)`.

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
