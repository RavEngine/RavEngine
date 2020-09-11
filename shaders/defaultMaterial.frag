#version 450

layout(location = 0) in vec3 varying_COLOR;
layout(location = 0) out vec4 out_var_SV_Target;

void main()
{
    out_var_SV_Target = vec4(varying_COLOR, 1.0);
}

