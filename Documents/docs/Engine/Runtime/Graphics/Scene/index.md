# Runtime.Graphics.Scene (Visera.Runtime.Graphics.Scene)

**Runtime.Graphics.Scene** provides scene representation: camera, lights and renderables. Camera defines view and projection matrices; lights define directional, point or spot parameters; renderables associate mesh, material and draw parameters for render graph and passes to traverse and issue draw calls. Scene data is filled by editor or game logic; render layer consumes read-only.

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
