/// Engine light data SSBO (Set 2, Binding 0).
/// CPU-side mirror: Visera::FGPULight.

struct GPULight
{
    uint   Type;          // 0=Directional, 1=Point, 2=Spot
    float3 Position;
    float3 Direction;
    float  Range;
    float4 Color;         // rgb + alpha (unused)
    float  Intensity;
    float  SpotAngle;
    uint   CastShadow;
    float  _Padding;
};

[[vk::binding(0, 2)]] StructuredBuffer<GPULight> _Lights;
