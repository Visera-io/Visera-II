// Visera Sprite Shader (Minimal)
// Matches Shader.schema: EntryPoints VertMain (Vertex), FragMain (Fragment);
// Resources: ConstantBuffer (Transform), Texture2D (BaseColor), SamplerState.
// Matches Material.schema: BaseColor texture binding.

struct VertexInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

struct TransformConstants
{
    float4x4 modelViewProj;
};

ConstantBuffer<TransformConstants> Transform : register(b0, space0);
Texture2D BaseColor : register(t1, space0);
SamplerState LinearSampler : register(s2, space0);

[shader("vertex")]
VertexOutput VertMain(VertexInput input)
{
    VertexOutput output;
    output.position = mul(Transform.modelViewProj, float4(input.position, 1.0));
    output.uv = input.uv;
    return output;
}

[shader("fragment")]
float4 FragMain(VertexOutput input) : SV_Target
{
    return BaseColor.Sample(LinearSampler, input.uv);
}
