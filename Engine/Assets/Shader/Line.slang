struct VSInput
{
    [[vk::location(0)]]
    float2 Pos;
};

struct VSOutput
{
    float4 Position : SV_Position;
};

[shader("vertex")]
VSOutput VertexMain(VSInput Input)
{
    VSOutput o;
    float2 ndc = float2(Input.Pos.x * 2.0 - 1.0,
                        Input.Pos.y * 2.0 - 1.0);
    o.Position = float4(ndc, 0.0, 1.0);
    return o;
}

struct PSInput
{
    float4 Position : SV_Position;
};

struct PushConstants
{
    float Time;
    float Alpha;
    float OffsetX, OffsetY;
};
[[vk::push_constant]] PushConstants PC;

[shader("fragment")]
float4 FragmentMain(PSInput input) : SV_Target
{
    float4 Color = float4(1.0, 0.0, 0.0, 1.0);
    return Color;
}
