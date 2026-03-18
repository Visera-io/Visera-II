/// Per-instance data for GPU instancing via StructuredBuffer.
/// CPU-side mirror: Visera::FInstanceData (FTransform3x4F + FVector4F + FVector4F = 80 bytes).

struct InstanceData
{
    float3x4 transform;   // 48 bytes -- affine model matrix (row-major, implicit 4th row = {0,0,0,1})
    float4   color;       // 16 bytes -- per-instance tint / modulation
    float4   customData;  // 16 bytes -- user-defined (UV rect, material params, etc.)
};

/// Instance buffer bound at set 3, binding 0.
[[vk::binding(0, 3)]]
StructuredBuffer<InstanceData> _instances;

/// Retrieve instance data for the current draw instance.
InstanceData GetInstance(uint instanceIndex)
{
    return _instances[instanceIndex];
}
