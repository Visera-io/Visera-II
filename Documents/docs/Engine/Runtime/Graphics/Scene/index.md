# Runtime.Graphics.Scene (Visera.Runtime.Graphics.Scene)

**Runtime.Graphics.Scene** provides the data types for rendering: camera (`FCamera`), lights (`FLight`) and renderables (`IRenderable`, `FRenderableMeta`). There is **no persistent scene container** (`FScene` was removed). Camera, lights and draws are submitted **per-frame** via `FGraphics::SetCamera`, `SubmitLight` and `Draw`; the engine calls `Render(FWindow*)` after `OnPreRender` to send the accumulated frame to the Graphics thread.

## Responsibilities
- **Camera**: View matrix (position and orientation), projection (perspective/ortho), viewport and culling; may support multi-viewport or split screen.
- **Light**: Type (directional/point/spot), color, intensity, attenuation, shadow; for lighting pass or deferred shading.
- **Renderable**: References to RHI buffers (vertex/index), pipeline and descriptor set, plus draw params (instance count, offset, etc.); extensible with material and skeleton.

## Submodules
| Module | Description |
|------|------|
| [Camera](Camera.md) | Camera and view/projection. |
| [Light](Light.md) | Light type and params. |
| [Renderable](Renderable.md) | Renderable and draw data. |

## See also
- [Graphics](../index.md) — Parent module
- [RenderGraph](../RenderGraph.md) — Render graph consumes scene
- [RHI](../../RHI/index.md) — Draw commands and resources
