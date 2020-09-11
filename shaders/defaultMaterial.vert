#version 450

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(binding = 0, std140) uniform type_Settings
{
    mat4 wvpMatrix;
} Settings;

layout(location = 0) in vec3 in_var_POSITION;
layout(location = 1) in vec3 in_var_COLOR;
layout(location = 0) out vec3 varying_COLOR;

void main()
{
    gl_Position = vec4(in_var_POSITION, 1.0) * Settings.wvpMatrix;
    varying_COLOR = in_var_COLOR;
}

