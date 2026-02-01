# Material

Visera Material defines shader, surface type, and texture bindings for rendering. The minimal format supports the **Sprite Renderer** via a single BaseColor texture and a shader identifier.

## Asset format

- **File extension**: `.vmaterial`
- **Schema**: `Engine/Schemas/Material.schema.json`

Required fields:

| Field | Type | Description |
|-------|------|--------------|
| `Version` | integer (≥1) | Material asset format version. |
| `Shader` | string | Shader identifier or path (e.g. `Sprite.slang`). |
| `Surface` | `"Opaque"` \| `"Masked"` \| `"Transparent"` | High-level surface type. |
| `Textures.BaseColor` | string | Base color texture asset path or id. |

Optional: `Name` (display name).

## Usage (Sprite Renderer)

1. **Load material** from a `.vmaterial` path:
   ```cpp
   FMaterial Mat(FPath{"Assets/Material/BasicSprite.vmaterial"});
   ```

2. **Resolve BaseColor texture**: load the image for `Mat.GetBaseColorPath()`, create a texture via `FGraphics::CreateTexture2D`, then set the handle on the material:
   ```cpp
   Mat.SetBaseColorHandle(Graphics->CreateTexture2D(Image));
   ```

3. **Use with Sprite Renderer**: set the material on `FSpriteRenderer`; the renderer uses `GetBaseColorHandle()` and `GetShader()` for drawing:
   ```cpp
   FSpriteRenderer Sprite;
   Sprite.SetMaterial(MakeShared<FMaterial>(FPath{"Assets/Material/BasicSprite.vmaterial"}));
   // After resolving BaseColor and setting on material, draw using Sprite.GetBaseColorHandle() and Sprite.GetShaderName().
   ```

## C++ API (minimal)

- **`FMaterial`**: load from path, get/set `Version`, `Shader`, `Surface`, `BaseColorPath`, `BaseColorHandle`; `IsValid()`.
- **`ESurfaceType`**: `Opaque`, `Masked`, `Transparent`.
- **`FSpriteRenderer`**: `SetMaterial` / `GetMaterial`, `GetBaseColorHandle()`, `GetShaderName()`.

Schema and code stay in sync: when the material format or assets change, update `Engine/Schemas/Material.schema.json` and this doc.
