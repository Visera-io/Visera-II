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

## FRenderableMeta

Plain struct holding extracted draw-command data. Used by `FGraphics::Draw(const FRenderableMeta&)` and by scripting bindings (no C++ virtuals). Data is captured at submit time so nothing crosses the thread boundary.

| Member        | Type                   | Description |
|---------------|------------------------|-------------|
| `InstanceData`| `FInstanceData`        | Transform, color, custom data. |
| `Material`    | `TSharedPtr<FMaterial>` | Material; required. |
| `Mesh`        | `TSharedPtr<FMesh>`     | Mesh geometry; `nullptr` = sprite quad path. |

## IRenderable

Abstract interface for C++ renderables. Implementations provide instance data, material and optional mesh. **`ToRenderableMeta()`** builds `FRenderableMeta` from the virtual getters; `FGraphics::Draw(const IRenderable&)` calls it and stores the result (no `TSharedPtr<IRenderable>` is kept).

| Method               | Return                    | Description |
|----------------------|---------------------------|-------------|
| `GetInstanceData()`  | `FInstanceData`           | **Pure virtual.** Returns the instance's transform, color and custom data. |
| `GetMaterial()`      | `TSharedPtr<FMaterial>`   | **Pure virtual.** Returns the material used by this renderable. |
| `GetMesh()`          | `TSharedPtr<FMesh>`       | **Virtual.** Returns mesh geometry; default `nullptr` uses the built-in sprite quad path (`Draw(6, N)`). Override to provide custom geometry (`DrawIndexed`). |
| `ToRenderableMeta()` | `FRenderableMeta`         | Builds meta from the three getters; used by `Draw(const IRenderable&)`. |

## Instancing pipeline

1. Game (or script) calls `GFX->Draw(renderable)` or `GFX->Draw(FRenderableMeta)` each frame; data is captured immediately.
2. After `OnPreRender`, the engine calls `GFX->Render(Window)` to send accumulated draws, camera and lights to the Graphics thread.
3. Graphics thread runs `BatchAndSort` on `FRenderData::DrawCommands` → `FRenderList` (batches by Material + Mesh).
4. `UploadInstanceBuffers` creates a per-batch SSBO and descriptor set at Set 1.
5. Draw passes bind Set 0 (material) + Set 1 (instance SSBO) and issue `Draw(6, N)` or `DrawIndexed(IndexCount, N)`.

## See also

- [Scene](index.md) — parent module
- [Camera](Camera.md) — camera
- [Framework](../Framework.md) — FRenderBatch and FRenderList
- [Material](../Material.md) — material system
- [Transform](../../../../Core/Math/Geometry/Transform.md) — FTransform3x4F
- [RHI.CommandList](../../RHI/CommandList.md) — command recording
