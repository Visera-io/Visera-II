const static float2 Vertices[3] =
{
        { -1.0,  3.0 },
        { -1.0, -1.0 },
        {  3.0, -1.0 },
};

const static float2 Vertices2[3] =
{
        { 0.0,   0.5 },
        { -0.5, -0.5 },
        {  0.5, -0.5 },
};

[shader("vertex")]
float4 VertexMain(uint _VertexID : SV_VertexID) : SV_Position
{
    return float4(Vertices[_VertexID], 0.0, 1.0);
}

//<< Fragment Shader>>
[shader("fragment")]
float4 FragmentMain2() : SV_Target
{
    return float4(1.0, 1.0, 1.0, 1.0);
}
///======================================================
 // Galaxy_NoUV.slang
 // Procedural galaxy swirl using SV_Position only
 //======================================================

 struct PSInput
 {
     float4 Position : SV_Position;
 };

 [shader("fragment")]
 float4 FragmentMain(PSInput input) : SV_Target
 {
     // Normalize pixel position to [-1, 1]
     float2 resolution = float2(1920.0, 1080.0);
     float2 uv = input.Position.xy / resolution * 2.0 - 1.0;
     uv.x *= resolution.x / resolution.y;

     float2 p = uv * 2.0;

     // Polar coordinates
     float r = length(p);
     float a = atan2(p.y, p.x);

     // Spiral pattern
     float spiral = sin(a * 3.0 + r * 6.0) * 0.5 + 0.5;

     // Star glow
     float star = exp(-10.0 * r * r);

     // Base galaxy color
     float3 col = float3(0.1, 0.0, 0.2);
     col += float3(0.4, 0.1, 0.8) * spiral * 0.8;
     col += float3(0.8, 0.8, 1.0) * star * 1.5;

     // Add soft vignetting
     col *= smoothstep(1.5, 0.3, r);

     return float4(col, 1.0);
 }