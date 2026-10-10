float4 constant0 : register(c0);
float4 constant91 : register(c91);

struct Input
{
    float4 attribute0 : POSITION;
    float4 attribute1 : TEXCOORD;
};

struct Output
{
    float4 color : COLOR0;
    float4 uv0 : TEXCOORD0;
    float4 position : POSITION;
    float4 specular : COLOR1;
    float fog : FOG;
};

Output main(Input input)
{
    Output output;
    output.color = constant0;
    output.uv0 = input.attribute1;
    output.position = input.attribute0;
    output.specular = constant91.x;
    output.fog = (constant91.z).x;
    return output;
}
