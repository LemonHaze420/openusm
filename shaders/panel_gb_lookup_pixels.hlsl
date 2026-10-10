sampler2D texture0 : register(s0);
sampler2D texture1 : register(s1);
float4 main(float2 uv : TEXCOORD0) : COLOR0
{
    return tex2D(texture1, tex2D(texture0, uv).gb);
}
