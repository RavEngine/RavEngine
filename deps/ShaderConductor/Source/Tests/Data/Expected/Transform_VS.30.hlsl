cbuffer type_cbVS : register(b0)
{
    row_major float4x4 cbVS_wvp : packoffset(c0);
};

uniform float4 gl_HalfPixel;

static float4 gl_Position;
static float4 in_var_POSITION;

struct SPIRV_Cross_Input
{
    float4 in_var_POSITION : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 gl_Position : POSITION;
};

void vert_main()
{
    gl_Position = mul(in_var_POSITION, cbVS_wvp);
    gl_Position.x = gl_Position.x - gl_HalfPixel.x * gl_Position.w;
    gl_Position.y = gl_Position.y + gl_HalfPixel.y * gl_Position.w;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    in_var_POSITION = stage_input.in_var_POSITION;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
