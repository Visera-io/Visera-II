# Runtime.Graphics (Visera.Runtime.Graphics)

**Runtime.Graphics** builds high-level graphics on top of RHI: framework init and lifecycle, material system, pipeline cache, render graph and scene (camera, light, renderable). Converts scene and materials to RHI commands and resource bindings; manages multi-pass and resource dependencies via render graph.

## Responsibilities
- **Framework**: Graphics subsystem init and global settings; coordination with RHI, window, etc.
- **Material**: Material type and params; binding to RHI pipeline, descriptor set and texture/sampler.
- **PipelineCache**: Pipeline object cache and persistence; faster startup and less recompile.
- **RenderGraph**: Declarative render graph; pass and resource dependencies; drives per-frame order and barriers.
- **Scene**: Camera (view/projection), lights (directional/point/spot), renderables (mesh and draw params) for render graph and passes.

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
