[shader("vertex")]
float4 VertexMain() : SV_Position
{
    return float4(0.0, 0.0, 0.0, 1.0);
}

struct PSInput
{
    float4 Position : SV_Position;
};

[shader("fragment")]
float4 FragmentMain(PSInput input) : SV_Target
{
    return float4(1.0, 0.0, 0.0, 1.0);
}
