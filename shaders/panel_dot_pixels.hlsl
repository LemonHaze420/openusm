float4 weights : register(c0);
sampler2D texture0 : register(s0);
float4 main(float2 uv : TEXCOORD0) : COLOR0
{
    return dot(tex2D(texture0, uv).rgb, weights.rgb);
}
