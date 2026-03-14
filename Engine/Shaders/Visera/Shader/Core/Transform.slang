/// Utility functions for affine transform matrices.

/// Expand a row-major float3x4 affine transform into a full float4x4
/// by appending the implicit bottom row (0, 0, 0, 1).
float4x4 ToMatrix4x4(float3x4 t)
{
    // t[0..2] are already float4 rows; just append the implicit 4th row.
    return float4x4(
        t[0],
        t[1],
        t[2],
        float4(0.0, 0.0, 0.0, 1.0)
    );
}
