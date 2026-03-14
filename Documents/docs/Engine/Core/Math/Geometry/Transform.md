# Core.Math.Geometry.Transform (Visera.Core.Math.Geometry.Transform)

3×4 affine transform type designed for GPU instancing. Memory layout matches Slang `float3x4` for zero-cost CPU→GPU transfer via SSBO.

## FTransform3x4F

Row-major 3×4 matrix stored as `FVector4F Rows[3]`. Each row is a `float4`; the implicit fourth row is `(0, 0, 0, 1)`.

`sizeof(FTransform3x4F) == 48`, `std::is_standard_layout_v == true`.

### Static factories

| Method | Parameters | Description |
|--------|-----------|-------------|
| `Identity()` | — | Returns the identity transform (rows = basis vectors + zero translation). |
| `FromMatrix4x4(I_Matrix)` | `const FMatrix4x4F&` | Extracts the top 3 rows from a column-major 4×4 matrix, transposing into row-major 3×4. |
| `MakeTransform2D(I_Position, I_Scale, I_Rotation, I_Depth)` | `FVector2F, FVector2F, FDegree, Float` | Builds a 2D affine transform: rotation around Z, non-uniform scale, translation in XY, depth in Z. |
| `MakeTransform3D(I_Position, I_Rotation, I_Scale)` | `const FVector3F&, const FQuaternion&, const FVector3F&` | Builds a 3D affine transform from position, quaternion rotation and scale. |

### Conversion

| Method | Return | Description |
|--------|--------|-------------|
| `ToMatrix4x4()` | `FMatrix4x4F` | Reconstructs a column-major 4×4 matrix by appending row `(0, 0, 0, 1)`. |

### Shader side

On the GPU, use `import Visera.Shader.Core;` to access:

- `InstanceData.transform` — `float3x4`, bit-identical to `FTransform3x4F`.
- `ToMatrix4x4(float3x4 t)` — helper in `Visera.Shader.Core.Transform`.

## See also

- [Geometry](index.md) — parent module
- [Vector](../Algebra/Vector.md) — FVector2F, FVector3F, FVector4F
- [Matrix](../Algebra/Matrix.md) — FMatrix4x4F
- [Quaternion](../Algebra/Quaternion.md) — FQuaternion
- [Trigonometry](../Trigonometry/index.md) — FDegree
- [Renderable](../../../../Runtime/Graphics/Scene/Renderable.md) — FInstanceData uses FTransform3x4F
