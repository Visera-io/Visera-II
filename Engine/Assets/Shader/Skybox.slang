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

//======================================================
// Galaxy_Rotating_Stars_Comets_Cascade.slang
// Rotating galaxy + stars + comet trail based on mouse offset
//======================================================

struct PSInput
{
    float4 Position : SV_Position;
};

struct PushConstants
{
    float time;
    float mouse_x, mouse_y;
    float offset_x, offset_y;
};
[[vk::push_constant]] PushConstants IO;

//-------------------------------------------
// Simple hash for random starfield
float hash(float2 p)
{
    p = frac(p * 0.3183099 + float2(0.1, 0.7));
    p *= 17.0;
    return frac(p.x * p.y * (p.x + p.y));
}

//-------------------------------------------
[shader("fragment")]
float4 FragmentMain(PSInput input) : SV_Target
{
    float2 resolution = float2(1920.0, 1080.0);
    float2 uv = input.Position.xy / resolution * 2.0 - 1.0;
    uv.x *= resolution.x / resolution.y;

    // --- 🌌 Galaxy background ---
    float2 p = uv * 2.0;
    float angle = IO.time * 0.2;
    float s = sin(angle), c = cos(angle);
    p = float2(c * p.x - s * p.y, s * p.x + c * p.y);
    float r = length(p);
    float a = atan2(p.y, p.x);
    float spiral = sin(a * 3.0 + r * 6.0) * 0.5 + 0.5;
    float starCore = exp(-10.0 * r * r);

    float3 col = float3(0.05, 0.0, 0.1);
    col += float3(0.4, 0.1, 0.8) * spiral * 0.8;
    col += float3(0.8, 0.8, 1.0) * starCore * 1.5;

    // --- 🌠 Starfield ---
    float2 starUV = uv * 10.0;
    float starNoise = hash(floor(starUV));
    float flicker = sin(IO.time * 3.0 + uv.x * 20.0 + uv.y * 30.0) * 0.5 + 0.5;
    float starMask = smoothstep(0.995, 1.0, starNoise) * flicker;
    col += starMask * float3(1.0, 1.0, 1.0);

    // --- ☄️ Comet logic ---
    float2 mouse = IO.mouse_x * 2.0 - 1.0;
    mouse.y = -(IO.mouse_y * 2.0 - 1.0);
    mouse.x *= resolution.x / resolution.y;

    // Tail direction from offset
    float2 offset = float2(IO.offset_x, -IO.offset_y);
    float offsetLen = clamp(length(offset) / 500.0, 0.0, 0.5); // normalize velocity
    float2 tailDir = (offsetLen > 0.001) ? normalize(offset) : float2(0.0, 1.0);

    // Main comet position (head)
    float2 cometPos = mouse;
    float cometDist = length(uv - cometPos);
    float cometHead = exp(-pow(cometDist * 20.0, 2.0));

    // Main tail
    float trail = exp(-abs(dot(uv - cometPos, -tailDir)) * 25.0)
                * exp(-length(uv - cometPos) * 10.0);

    float comet = cometHead * 1.2 + trail * 0.8;
    comet *= 0.8 + 0.2 * sin(IO.time * 6.0);

    // --- ✨ Cascaded tail effects ---
    float3 cometColor = float3(0.7, 0.9, 1.0);
    float3 totalComet = comet * cometColor;

    // Add multiple faded "ghosts" trailing behind the cursor
    const int TAIL_COUNT = 5;
    for (int i = 1; i <= TAIL_COUNT; ++i)
    {
        float fade = 1.0 - float(i) / (TAIL_COUNT + 1);
        float timeShift = IO.time - i * 0.03;
        float2 trailOffset = -tailDir * (float(i) * offsetLen * 3.0);

        float2 ghostPos = mouse + trailOffset;
        float ghostDist = length(uv - ghostPos);
        float ghostHead = exp(-pow(ghostDist * 20.0, 2.0)) * fade;
        float ghostTrail = exp(-abs(dot(uv - ghostPos, -tailDir)) * 25.0)
                         * exp(-length(uv - ghostPos) * 10.0) * fade;
        float ghost = ghostHead + ghostTrail;

        totalComet += ghost * cometColor * (0.3 + 0.7 * fade);
    }

    // --- 🌘 Apply galaxy fade ---
    float vignette = smoothstep(1.6, 0.3, r);
    float breathing = 0.8 + 0.2 * sin(IO.time * 0.5);
    col *= vignette * breathing;

    // --- ☄️ Add comet cascades ---
    col += totalComet;

    // Optional: glowing aura
    float halo = exp(-pow(cometDist * 8.0, 2.0));
    //col += halo * float3(0.3, 0.6, 1.0);
    col += halo * float3(1.0 * IO.offset_x, 0.6, 1.0);

    return float4(col, 1.0);
}