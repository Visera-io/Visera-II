struct VSInput
{
    [[vk::location(0)]]
    float4 Data;
};

struct VSOutput
{
    float4 Position : SV_Position;
    float2 UV       : TEXCOORD0;
};

[shader("vertex")]
VSOutput VertexMain(VSInput Input)
{
    VSOutput o;
    o.Position = float4(Input.Data.x, Input.Data.y, 0.0, 1.0);
    o.UV = Input.Data.zw;
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
    float Alpha;
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
    return float4(Color.xyz, Color.w * PC.Alpha);
}
