/// Engine per-frame uniform data (Set 0, Binding 0).
/// CPU-side mirror: Visera::FEngineFrameData.

struct EngineFrameData
{
    float4x4 ViewMatrix;
    float4x4 ProjectionMatrix;
    float4x4 ViewProjectionMatrix;
    float4   RenderSize;       // xy = width/height, zw = 1/width, 1/height
    float4   UserData0;        // app-provided: interpretation is shader-specific
    float4   UserData1;
    float4   UserData2;
    float4   UserData3;
};

[[vk::binding(0, 0)]] ConstantBuffer<EngineFrameData> _Frame;
