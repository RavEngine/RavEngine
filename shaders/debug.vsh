layout(location = 0) in vec3 inPosition;
layout(location = 1) in uint inColor;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform UniformBufferObject{
	mat4 modelViewProj;
	ivec4 viewRect;
} ubo;


void main()
{
	gl_Position = ubo.modelViewProj * vec4(inPosition, 1.0);
	outColor = unpackUnorm4x8(inColor);
}
