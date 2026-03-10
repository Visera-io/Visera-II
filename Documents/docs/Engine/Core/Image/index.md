# Core.Image (Visera.Core.Image)

**Core.Image** provides image-related types and utilities: common image data structures, pixel format and layout. Used by [Runtime.AssetHub](../../Runtime/AssetHub/index.md) image loading and [RHI](../../Runtime/RHI/index.md) texture creation for a unified 2D image and pixel format representation (e.g. RGBA, sRGB, linear).

## Responsibilities
- **Common**: Shared image size, stride, format enums across modules to avoid duplicate definitions.
- **Pixel**: Pixel format and channel layout (bytes per pixel, channel order), aligned with RHI texture format or shader sampling.

## Submodules
| Module | Description |
|------|------|
| [Common](Common.md) | Common image types and helpers. |
| [Pixel](Pixel.md) | Pixel format and types. |

## See also
- [Core](../index.md) — Parent module
- [Font](../Font/index.md) — Font rasterization may produce images
- [Runtime.AssetHub.Image](../../Runtime/AssetHub/Image/index.md) — Runtime image loading
