layout(location = 0) in vec4 inPosition;		// w (unused) is the size
layout(location = 1) in uint inColor;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform UniformBufferObject{
	mat4 viewProj;
	ivec4 viewRect;
} ubo;


void main()
{
	gl_Position = ubo.viewProj * vec4(inPosition.xyz, 1.0);
	outColor = unpackUnorm4x8(inColor);
}
