
layout(push_constant) uniform UniformBufferObject{
    mat4 model;
	mat4 viewProj;
} ubo;

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texcoord;
layout(location = 2) in uint a_color;

layout(location = 0) out vec2 v_texcoord;
layout(location = 1) out vec4 v_color;

void main()
{
    vec4 pos = ubo.model * vec4(a_position, 1.0);
    gl_Position = ubo.viewProj * pos;
    gl_PointSize = 1;

    v_color = unpackUnorm4x8(a_color);
    v_texcoord = a_texcoord;
}
