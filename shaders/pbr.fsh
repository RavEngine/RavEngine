
layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec4 outcolor;
layout(location = 1) out vec4 outnormal;

layout(binding = 0) uniform sampler2D diffuseSampler; 

layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
	vec4 colorTint;
} ubo;

void main()
{
	outcolor = texture(diffuseSampler, inUV) * ubo.colorTint;
	outnormal = vec4(inNormal, 1);
}

