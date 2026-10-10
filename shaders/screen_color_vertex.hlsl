float4 constant91 : register(c91);

struct Input
{
    float4 attribute0 : POSITION;
    float4 attribute1 : COLOR;
    float4 attribute2 : TEXCOORD;
};

struct Output
{
    float4 position : POSITION;
    float4 color : COLOR0;
    float4 uv0 : TEXCOORD0;
    float4 specular : COLOR1;
    float fog : FOG;
};

Output main(Input input)
{
    Output output;
    output.position = input.attribute0;
    output.color = input.attribute1;
    output.uv0 = input.attribute2;
    output.specular = constant91.x;
    output.fog = (constant91.z).x;
    return output;
}
