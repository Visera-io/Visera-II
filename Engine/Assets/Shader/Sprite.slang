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

// --- ShaderToy-style helpers ----------------------------------------------
static const float PI = 3.14159265359;

float hash21(float2 p)
{
    p = frac(p * float2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return frac(p.x * p.y);
}

float2 rot2(float2 p, float a)
{
    float s = sin(a), c = cos(a);
    return float2(c * p.x - s * p.y, s * p.x + c * p.y);
}

float valueNoise(float2 p)
{
    float2 i = floor(p);
    float2 f = frac(p);
    float a = hash21(i);
    float b = hash21(i + float2(1, 0));
    float c = hash21(i + float2(0, 1));
    float d = hash21(i + float2(1, 1));
    float2 u = f * f * (3.0 - 2.0 * f);
    return lerp(lerp(a, b, u.x), lerp(c, d, u.x), u.y);
}

// IQ-ish equilateral triangle SDF
float sdEquilateralTriangle(float2 p)
{
    const float k = 1.7320508075688772; // sqrt(3)
    p.x = abs(p.x) - 1.0;
    p.y = p.y + 1.0 / k;
    if (p.x + k * p.y > 0.0)
        p = float2(p.x - k * p.y, -k * p.x - p.y) * 0.5;
    p.x -= clamp(p.x, -2.0, 0.0);
    return -length(p) * sign(p.y);
}

// A thin neon line from SDF distance
float neonLine(float d, float w)
{
    // w: width in SDF units
    float a = smoothstep(w, 0.0, abs(d));
    float b = smoothstep(w * 6.0, 0.0, abs(d));
    return a + b * 0.35; // core + bloom
}

// --- New cooler background fragment entry ---------------------------------

[shader("fragment")]
float4 FragmentBackgroundMain(PSInput input) : SV_Target
{
    float2 uv = input.UV;

    // reconstruct resolution from derivatives
    float w = 1.0 / max(abs(ddx(uv.x)), 1e-6);
    float h = 1.0 / max(abs(ddy(uv.y)), 1e-6);
    float2 iResolution = float2(w, h);
    float2 fragCoord   = uv * iResolution;

    float  t = PC.Time;
    float2 p = (fragCoord / iResolution) * 2.0 - 1.0;
    p.x *= (iResolution.x / iResolution.y);

    // subtle external offsets (handy for camera drift/shake)
    p += float2(PC.OffsetX, PC.OffsetY) * 0.02;

    // ---------------- Background: diagonal speed + parallax noise ----------
    float2 drift = float2(0.22, 0.33) * t;            // up-right
    float2 q = p + drift;

    // streaks: diagonal bands with time-warp
    float bands = sin((q.x + q.y) * 10.0 + sin(t * 0.7) * 1.2);
    bands = 0.5 + 0.5 * bands;
    bands = pow(bands, 2.0);

    // parallax fog layers
    float fog1 = valueNoise(q * 2.0 + float2(t * 0.10, t * 0.08));
    float fog2 = valueNoise((q + float2(0.6, -0.3)) * 4.0 + float2(t * 0.18, t * 0.14));
    float fog  = saturate(0.65 * fog1 + 0.35 * fog2);
    fog = fog * fog;

    // base palette: deep blue -> purple
    float3 bgA = float3(0.02, 0.03, 0.06);
    float3 bgB = float3(0.07, 0.05, 0.12);
    float3 color = lerp(bgA, bgB, saturate(0.35 * bands + 0.65 * fog));

    // subtle grain
    color += (hash21(fragCoord + t) - 0.5) * 0.01;

    // vignette
    float vign = smoothstep(1.45, 0.25, length(p));
    color *= lerp(0.88, 1.08, vign);

    // ---------------- Hero: neon energy triangle ---------------------------
    // triangle transform
    float2 tp = p;
    tp = rot2(tp, t * 0.55);     // slow rotation
    tp.y += 0.04;
    tp *= 0.78;

    float d = sdEquilateralTriangle(tp);

    // animated width (breathing)
    float breath = 0.006 + 0.0025 * (0.5 + 0.5 * sin(t * 2.2));
    float edge = neonLine(d, breath);

    // inner "energy" fill near edges + center glow
    float inside = smoothstep(0.06, 0.0, d);          // inside mask
    float rim    = smoothstep(0.03, 0.0, d) - smoothstep(0.10, 0.0, d);
    float centerGlow = exp(-3.2 * max(d, 0.0)) * 0.12;

    // electric arc modulation along edges
    float arc = valueNoise(tp * 12.0 + float2(t * 1.7, -t * 1.3));
    arc = pow(arc, 3.0);
    float arcBoost = 0.6 + 0.8 * arc;

    // scanline / sweep across triangle
    float sweep = 0.5 + 0.5 * sin((tp.x * 3.0 - tp.y * 4.0) + t * 6.0);
    sweep = pow(sweep, 6.0);

    // neon palette
    float3 neonC = float3(0.25, 0.95, 1.10);  // cyan
    float3 neonM = float3(1.05, 0.25, 0.90);  // magenta

    // gradient mix based on angle
    float ang = atan2(tp.y, tp.x);
    float g = 0.5 + 0.5 * sin(ang * 2.0 + t);
    float3 triCol = lerp(neonC, neonM, g);

    // compose triangle light
    float triLight =
        edge * arcBoost +
        rim * (0.35 + 0.65 * sweep) +
        centerGlow;

    // flicker (subtle, not annoying)
    float flicker = 0.92 + 0.08 * sin(t * 28.0 + hash21(floor(fragCoord * 0.7)) * 6.2831);
    triLight *= flicker;

    // add bloom-ish contribution
    color += triCol * triLight * 0.55;

    // soft halo around triangle (outside glow)
    float outer = smoothstep(0.20, 0.0, abs(d));
    color += triCol * outer * 0.06;

    // clamp and output
    color = saturate(color);
    return float4(color, PC.Alpha);
}

