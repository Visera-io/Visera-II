# Runtime.Graphics.Scene.Renderable (Visera.Runtime.Graphics.Scene.Renderable)

Defines the per-instance data model, mesh abstraction, and renderable interface used by the scene and draw-list extraction.

## FInstanceData

Unified per-instance data for both 2D and 3D renderables. Uploaded to the GPU as a storage buffer (SSBO) and accessed in shaders via `StructuredBuffer<InstanceData>` (Set 1, Binding 0).

| Member      | Type              | Description |
|-------------|-------------------|-------------|
| `Transform` | `FTransform3x4F`  | 3×4 affine transform (row-major, matches Slang `float3x4`). |
| `Color`     | `FVector4F`       | Per-instance color / tint (RGBA). |
| `CustomData`| `FVector4F`       | Shader-specific data (e.g. UV origin + extent for sprites). |

`sizeof(FInstanceData) == 80`, standard layout.

## FMesh

GPU mesh data stored in storage buffers. The PSO has **no vertex input state**; all vertex data is fetched by the shader via SSBO (`StructuredBuffer<FVertex>`). Index data uses a traditional index buffer bound via `BindIndexBuffer`.

| Member        | Type            | Description |
|---------------|-----------------|-------------|
| `VertexBuffer`| `FRHIBufferID`  | Storage buffer holding vertex data. |
| `IndexBuffer` | `FRHIBufferID`  | Index buffer. |
| `VertexCount` | `UInt32`        | Number of vertices. |
| `IndexCount`  | `UInt32`        | Number of indices. |
| `IndexType`   | `ERHIIndexType` | `UInt16` or `UInt32`. |

## IRenderable

Abstract interface implemented by all scene objects that contribute draw calls.

| Method             | Return                    | Description |
|--------------------|---------------------------|-------------|
| `GetInstanceData()`| `FInstanceData`           | **Pure virtual.** Returns the instance's transform, color and custom data. |
| `GetMaterial()`    | `TSharedPtr<FMaterial>`   | **Pure virtual.** Returns the material used by this renderable. |
| `GetMesh()`        | `TSharedPtr<FMesh>`       | **Virtual.** Returns mesh geometry; default `nullptr` uses the built-in sprite quad path (`Draw(6, N)`). Override to provide custom geometry (`DrawIndexed`). |

## Instancing pipeline

1. Game code creates `IRenderable` implementations with `FInstanceData` built from `FTransform3x4F::MakeTransform2D` / `MakeTransform3D`.
2. `ExtractAndSortDrawList` groups renderables into `FRenderBatch` by **Pipeline + Material + Mesh**.
3. `UploadInstanceBuffers` creates a per-batch SSBO from `FInstanceData[]` and a descriptor set at Set 1.
4. Draw passes bind Set 0 (material) + Set 1 (instance SSBO) and issue `Draw(6, N)` or `DrawIndexed(IndexCount, N)`.

## See also

- [Scene](index.md) — parent module
- [Camera](Camera.md) — camera
- [Framework](../Framework.md) — FRenderBatch and FRenderList
- [Material](../Material.md) — material system
- [Transform](../../../../Core/Math/Geometry/Transform.md) — FTransform3x4F
- [RHI.CommandList](../../RHI/CommandList.md) — command recording
