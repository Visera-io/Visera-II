# Runtime.Graphics.Material (Visera.Runtime.Graphics.Material)

Material system: loads `.vmat` JSON definitions, creates descriptor sets for textures/samplers, and exposes render state for PSO creation.

## FMaterial

A material owns its shader references, descriptor set (set 0), and render state parsed from JSON.

### Render state

| Property         | Type            | JSON Key           | Default      | Description |
|------------------|-----------------|--------------------|--------------|-------------|
| `SurfaceType`    | `ESurfaceType`  | `Surface`          | `Opaque`     | Opaque / Transparent. |
| `CullMode`       | `ERHICullMode`  | `State.CullMode`   | `Back`       | Back / Front / None. |
| `bDepthTest`     | `Bool`          | `State.DepthTest`  | `false`      | Enable depth testing. |
| `bZWrite`        | `Bool`          | `State.DepthWrite` | `false`      | Enable depth writes. |
| `DepthCompareOp` | `ERHICompareOp` | `State.DepthCompare`| `Less`      | Comparison operation for depth test. |

Depth state is forwarded to `FPipelineCache::GetOrCreate` and baked into the PSO's `DepthStencilState`.

### Descriptor set 0 (material resources)

The material's descriptor set is created from **shader set 0 resources only**. During reflection merge, resources belonging to set 1+ (e.g. the engine-managed instance SSBO) are filtered out by the `AddResource` lambda (`if (Res.Set != 0) { return; }`). This prevents binding collisions between material textures (set 0, binding 0 = `SampledImage`) and the instance buffer (set 1, binding 0 = `StorageBuffer`).

### JSON format (`.vmat`)

```json
{
  "Surface": "Opaque",
  "Shaders": {
    "Vertex":   "Sprite.VertMain",
    "Fragment": "Sprite.FragMain"
  },
  "State": {
    "CullMode": "None",
    "DepthTest": true,
    "DepthWrite": true,
    "DepthCompare": "LessOrEqual"
  },
  "Textures": {
    "BaseColor": { "Image": "MyTexture", "Sampler": { ... } }
  }
}
```

## See also

- [Graphics](index.md) — parent module
- [PipelineCache](PipelineCache.md) — pipeline cache (consumes material state)
- [Renderable](Scene/Renderable.md) — FInstanceData and FMesh
- [RHI.Common](../RHI/Common.md) — ERHICompareOp, ERHICullMode
