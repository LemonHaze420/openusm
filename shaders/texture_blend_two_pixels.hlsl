sampler2D texture0 : register(s0);
sampler2D texture1 : register(s1);
float4 main(float2 uv0 : TEXCOORD0, float2 uv1 : TEXCOORD1, float4 color0 : COLOR0, float4 color1 : COLOR1) : COLOR0
{
    return tex2D(texture1, uv1) * color1.a + tex2D(texture0, uv0) * color0.a;
}
