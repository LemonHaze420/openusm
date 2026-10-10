sampler2D texture0 : register(s0);
sampler2D texture1 : register(s1);
float4 main(float2 uv0 : TEXCOORD0, float2 uv1 : TEXCOORD1, float4 color : COLOR0) : COLOR0
{
    float4 base = tex2D(texture0, uv0);
    float4 layer = tex2D(texture1, uv1);
    return lerp(base, layer, layer.a) * color;
}
