float4 constant0 : register(c0);
float4 constant1 : register(c1);
float4 constant2 : register(c2);
float4 constant3 : register(c3);

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
};

Output main(Input input)
{
    Output output;
    output.position.x = dot((input.attribute0).xyzw, (constant0).xyzw);
    output.position.y = dot((input.attribute0).xyzw, (constant1).xyzw);
    output.position.z = dot((input.attribute0).xyzw, (constant2).xyzw);
    output.position.w = dot((input.attribute0).xyzw, (constant3).xyzw);
    output.color = input.attribute1;
    output.uv0 = input.attribute2;
    return output;
}
