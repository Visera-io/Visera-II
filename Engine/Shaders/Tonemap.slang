struct Params
{
    uint  width;
    uint  height;
    float exposure;   // e.g. 1.0 = no change, or 2^EV
    uint  srgbOut;    // 0: output linear, 1: output sRGB-encoded
};

[[vk::push_constant]]
Params gParams;

[[vk::binding(0, 0)]]
Texture2D<float4>        gHDR;
[[vk::binding(1, 0)]]
RWTexture2D<float4>      gLDR;

static float3x3 ACESInputMat = float3x3(
    0.59719, 0.35458, 0.04823,
    0.07600, 0.90834, 0.01566,
    0.02840, 0.13383, 0.83777
);

static float3x3 ACESOutputMat = float3x3(
     1.60475, -0.53108, -0.07367,
    -0.10208,  1.10813, -0.00605,
    -0.00327, -0.07276,  1.07602
);

float3 RRTAndODTFit(float3 v)
{
    // Fitted curve: good compromise of filmic rolloff and stability
    float3 a = v * (v + 0.0245786) - 0.000090537;
    float3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

float3 TonemapACESFitted(float3 color)
{
    // ACES fitted expects linear input, in "sRGB-like" working space.
    color = mul(ACESInputMat, color);
    color = RRTAndODTFit(color);
    color = mul(ACESOutputMat, color);
    return saturate(color);
}

float3 SRGBEncode(float3 linearRGB)
{
    // Avoid NaNs for negative values (can happen after math / bloom)
    linearRGB = max(linearRGB, 0.0);

    float3 lo = linearRGB * 12.92;
    float3 hi = 1.055 * pow(linearRGB, 1.0 / 2.4) - 0.055;
    return lerp(hi, lo, step(linearRGB, 0.0031308));
}

[numthreads(16, 16, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    uint2 xy = dtid.xy;
    if (xy.x >= gParams.width || xy.y >= gParams.height) return;

    float4 hdr = gHDR.Load(int3(xy, 0));

    // 1) Exposure (simple)
    float3 color = hdr.rgb * gParams.exposure;

    // 2) ACES tonemap
    float3 ldr = TonemapACESFitted(color);

    // 3) Optional gamma encode
    if (gParams.srgbOut != 0)
        ldr = SRGBEncode(ldr);

    gLDR[xy] = float4(ldr, 1.0);
}