
layout(location = 0) in vec2 a_position;
layout(location = 1) in uint a_color0;
layout(location = 2) in vec2 a_texcoord0;

layout(location = 0) out vec4 v_color0;
layout(location = 1) out vec2 v_texcoord0;

layout(push_constant) uniform UniformBufferObject{
	mat4 modelMat;
} ubo;

void main()
{
	v_color0 = unpackUnorm4x8(a_color0);
	v_texcoord0	= a_texcoord0;
	gl_Position = ubo.modelMat * vec4(a_position, 0, 1);
	gl_Position.z = 1;
}
