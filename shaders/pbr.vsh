#version 450

layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
} ubo;


layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

void main()
{
	gl_Position = ubo.viewProj * vec4(a_position, 1);
}
