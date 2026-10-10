float4 constant91 : register(c91);

struct Input
{
    float4 attribute0 : POSITION;
    float4 attribute1 : TEXCOORD;
    float4 attribute2 : TEXCOORD1;
    float4 attribute3 : TEXCOORD2;
    float4 input4 : TEXCOORD3;
};

struct Output
{
    float4 position : POSITION;
    float4 uv0 : TEXCOORD0;
    float4 uv1 : TEXCOORD1;
    float4 uv2 : TEXCOORD2;
    float4 uv3 : TEXCOORD3;
    float4 specular : COLOR1;
    float fog : FOG;
};

Output main(Input input)
{
    Output output;
    output.position = input.attribute0;
    output.uv0 = input.attribute1;
    output.uv1 = input.attribute2;
    output.uv2 = input.attribute3;
    output.uv3 = input.input4;
    output.specular = constant91.x;
    output.fog = (constant91.z).x;
    return output;
}
