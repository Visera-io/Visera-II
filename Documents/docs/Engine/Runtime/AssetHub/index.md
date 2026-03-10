# Runtime.AssetHub (Visera.Runtime.AssetHub)

**Runtime.AssetHub** provides runtime asset loading and management: image (PNG, EXR, etc.), font (FreeType), shader, and generic Asset type and lifecycle. Load results feed RHI resources (texture, buffer) or graphics pipeline (shader, material) for Graphics and game logic.

## Responsibilities
- **Asset**: Generic asset type and load/unload interface; base or concept for concrete assets.
- **Image**: Load image from disk or memory; PNG, EXR and common formats; Common type and Wrapper for RHI texture.
- **Font**: Font load and rasterization; FreeType backend; for UI or text rendering.
- **Shader**: Shader asset load (source or bytecode) for RHI and pipeline cache.

## Submodules
| Module | Description |
|------|------|
| [Asset](Asset.md) | Generic Asset type and lifecycle. |
| [Image](Image/index.md) | Image loading: Common, PNG, EXR, Wrapper. |
| [Font](Font/index.md) | Font loading; [FreeType](Font/FreeType.md) implementation. |
| [Shader](Shader.md) | Shader asset loading. |

## See also
- [Runtime](../index.md) — Parent module
- [RHI.Resource](../RHI/Resource/index.md) — Texture and shader resources
- [Core.Image](../../Core/Image/index.md) — Core image and pixel types
