float4 constant0 : register(c0);
float4 constant1 : register(c1);
float4 constant2 : register(c2);
float4 constant3 : register(c3);
float4 constant4 : register(c4);
float4 constant91 : register(c91);

struct Input
{
    float4 attribute0 : POSITION;
};

struct Output
{
    float4 position : POSITION;
    float4 color : COLOR0;
    float4 specular : COLOR1;
    float fog : FOG;
};

Output main(Input input)
{
    Output output;
    output.position.x = dot((input.attribute0).xyzw, (constant0).xyzw);
    output.position.y = dot((input.attribute0).xyzw, (constant1).xyzw);
    output.position.z = dot((input.attribute0).xyzw, (constant2).xyzw);
    output.position.w = dot((input.attribute0).xyzw, (constant3).xyzw);
    output.color = constant4;
    output.specular = constant91.x;
    output.fog = (constant91.z).x;
    return output;
}
