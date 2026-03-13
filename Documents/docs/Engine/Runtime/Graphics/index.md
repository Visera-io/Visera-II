# Runtime.Graphics (Visera.Runtime.Graphics)

**Runtime.Graphics** builds high-level graphics on top of RHI: framework init and lifecycle, material system, pipeline cache, render graph and scene (camera, light, renderable). Converts scene and materials to RHI commands and resource bindings; manages multi-pass and resource dependencies via render graph.

## Responsibilities
- **Framework**: Graphics subsystem init; `FRenderTask` / `FRenderContext`; **draw list** extraction (`ExtractAndSortDrawList` → `FRenderList` by material/PSO); pass registration with read/write lock for concurrency.
- **Material**: Material type and params; binding to RHI pipeline, descriptor set and texture/sampler.
- **PipelineCache**: Get-or-create PSO by material and formats; **hash-based O(1)** lookup (GoldenRatio); collision handled by full key comparison.
- **RenderGraph**: Declarative passes; **Execute(FRenderContext)** (creates command list, runs passes with `RenderList`, submits); pass names as `FName`; **CullDeadPasses** O(N+E) reverse BFS.
- **Scene**: Camera (view/projection), lights, renderables; scene data is consumed once per frame to build `FRenderList`, then draw passes use only the list.

## Submodules
| Module | Description |
|------|------|
| [Framework](Framework.md) | Graphics framework init and config. |
| [Material](Material.md) | Material type and binding. |
| [PipelineCache](PipelineCache.md) | Pipeline cache. |
| [RenderGraph](RenderGraph.md) | Render graph. |
| [Scene](Scene/index.md) | [Camera](Scene/Camera.md), [Light](Scene/Light.md), [Renderable](Scene/Renderable.md). |

## See also
- [Runtime](../index.md) — Parent module
- [RHI](../RHI/index.md) — Low-level render interface
- [AssetHub](../AssetHub/index.md) — Texture and shader asset loading
