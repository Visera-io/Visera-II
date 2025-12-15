// CubeProcedural.slang

struct VSOutput
{
    float4 Position : SV_Position;
};

[shader("vertex")]
VSOutput VertexMain(uint vid : SV_VertexID)
{
    VSOutput o;
    float2 pos;

    // Fullscreen triangle
    if (vid == 0)      pos = float2(-1.0, -1.0);
    else if (vid == 1) pos = float2( 3.0, -1.0);
    else               pos = float2(-1.0,  3.0);

    o.Position = float4(pos, 0.0, 1.0);
    return o;
}

// 固定分辨率
static const float2 kScreenSize = float2(1920.0, 1080.0);

struct PushConstants
{
    float2 CubeCenter;   // 像素
    float2 CubeHalfSize; // 像素
    float  CubeAngle;    // rad
    float  Time;         // s
    float2 MousePos;     // 像素
    float2 _Pad;         // 对齐用
};
[[vk::push_constant]] PushConstants PC;

[shader("fragment")]
float4 FragmentMain(VSOutput input) : SV_Target
{
    float2 frag = input.Position.xy;          // 像素

    float2 d = frag - PC.CubeCenter;

    float c = cos(-PC.CubeAngle);
    float s = sin(-PC.CubeAngle);
    float2 local;
    local.x =  c * d.x - s * d.y;
    local.y =  s * d.x + c * d.y;

    bool inside =
        abs(local.x) <= PC.CubeHalfSize.x &&
        abs(local.y) <= PC.CubeHalfSize.y;

    // ⭐ 关键：立方体外面直接丢弃，不写颜色
    if (!inside)
        discard;

    // 下面保持原来的着色逻辑
    static const float2 kScreenSize = float2(1920.0, 1080.0);
    float2 mouseNdc = (PC.MousePos / kScreenSize) * 2.0 - 1.0;
    float  mouseDist = length(mouseNdc);
    float  pulse = 0.5 + 0.5 * sin(PC.Time * 1.5);
    float3 baseColor = float3(0.7, 0.45, 0.25);
    float  factor = saturate(1.0 - mouseDist * 0.6);

    return float4(baseColor * pulse * factor, 1.0);
}

