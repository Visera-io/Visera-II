struct PushConstants
{
    uint TextureHeight;
    uint TextureWidth;
    uint Stide;
};
[[vk::push_constant]]
PushConstants PC;

struct VSOutput
{
    float4 Corners01 : TEXCOORD0;
    float4 Corners23 : TEXCOORD1;
    float4 Center    : SV_Postion;
    float4 Corners56 : TEXCOORD2;
    float4 Corners78 : TEXCOORD3;
};

[shader("vertex")]
VSOutput VertexMain()
{
    VSOutput O;
    O.Center = //WIP
    O.Corners01 = O.Center + float4(-PC.Stide, -PC.Stide,         0, -PC.Stide);
    O.Corners23 = O.Center + float4( PC.Stide, -PC.Stide, -PC.Stide,  0);
    O.Corners56 = O.Center + float4( PC.Stide,         0, -PC.Stide,  PC.Stide);
    O.Corners78 = O.Center + float4(        0,  PC.Stide,  PC.Stide,  PC.Stide);

    return O;
}

struct PSInput
{
    float4 Corners01 : TEXCOORD0;
    float4 Corners23 : TEXCOORD1;
    float4 Center    : SV_Postion;
    float4 Corners56 : TEXCOORD2;
    float4 Corners78 : TEXCOORD3;
};

[[vk::binding(0, 0)]]
Sampler2D ReadTexture;

[shader("fragment")]
float4 FragmentMain(PSInput Input) : SV_Target
{
    SomeType Information ;
    Information = tex2D ( readTex , Coord01 .xy); // neighbor 0
    Compute a criterion value using Information ;
    Information = tex2D ( readTex , Coord01 .zw); // neighbor 1
    Compute a criterion value using Information ;
    Information = tex2D ( readTex , Coord23 .xy); // neighbor 2
    Compute a criterion value using Information ;
    Information = tex2D ( readTex , Coord23 .zw); // neighbor 3
    Compute a criterion value using Information ;
    Information = tex2D ( readTex , position .xy); // itself
    Compute a criterion value using Information ;
    Information = tex2D ( readTex , Coord56 .xy); // neighbor 5
    Compute a criterion value using Information ;s
    Information = tex2D ( readTex , Coord56 .zw); // neighbor 6
    Compute a criterion value using Information ;
    Information = tex2D ( readTex , Coord78 .xy); // neighbor 7
    Compute a criterion value using Information ;
    Information = tex2D ( readTex , Coord78 .zw); // neighbor 8
    Compute a criterion value using Information ;
    float4 bestInfo ;
    Using the criterion values to select the best seed , and
    store its Information into bestInfo ;
    oColor = bestInfo ;

    float4 Color = float4(0.0, 1.0, 0.0, 1.0); // Green when debug.
    return Color;
}
