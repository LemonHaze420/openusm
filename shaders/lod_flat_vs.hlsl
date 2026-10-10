float4 constant8 : register(c8);
float4 constant11 : register(c11);
float4 constant12 : register(c12);
float4 constant13 : register(c13);
float4 constant14 : register(c14);

struct Input
{
    float4 attribute0 : POSITION;
};

struct Output
{
    float4 position : POSITION;
    float4 color : COLOR0;
};

Output main(Input input)
{
    Output output;
    output.position.x = dot((input.attribute0).xyzw, (constant11).xyzw);
    output.position.y = dot((input.attribute0).xyzw, (constant12).xyzw);
    output.position.z = dot((input.attribute0).xyzw, (constant13).xyzw);
    output.position.w = dot((input.attribute0).xyzw, (constant14).xyzw);
    output.color = constant8;
    return output;
}
