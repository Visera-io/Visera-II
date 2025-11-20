const static float2 Vertices[6] =
{
        { -1.0, -1.0 },
        {  1.0, -1.0 },
        {  1.0,  1.0 },
        {  1.0,  1.0 },
        { -1.0,  1.0 },
        { -1.0, -1.0 },
};

const static float2 UVs[6] =
{
        { 0.0, 0.0 },
        { 1.0, 0.0 },
        { 1.0, 1.0 },
        { 1.0, 1.0 },
        { 0.0, 1.0 },
        { 0.0, 0.0 },
};

struct VSOutput
{
    float4 Position : SV_Position;
    float2 UV       : TEXCOORD0;
};


[shader("vertex")]
VSOutput VertexMain(uint _VertexID : SV_VertexID)
{
    VSOutput o;
    o.Position = float4(Vertices[_VertexID], 0.0, 1.0);
    o.UV = UVs[_VertexID];
    return o;
}


struct PSInput
{
    float4 Position : SV_Position;
    float2 UV       : TEXCOORD0;
};

struct PushConstants
{
    float Time;
    float Scale;
    float OffsetX, OffsetY;
};
[[vk::push_constant]] PushConstants PC;

[[vk::binding(0, 0)]]
Sampler2D FairyTex;

[shader("fragment")]
float4 FragmentMain(PSInput input) : SV_Target
{
    float2 uv = input.UV;
    // If your PNG appears upside-down, uncomment the next line:
    // uv.y = 1.0 - uv.y;
    float4 Color = FairyTex.Sample(uv);
    return float4(Color.xyz, Color.w * 0.2);
}
