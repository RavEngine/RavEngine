
layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec4 outcolor;
layout(location = 1) out vec4 outnormal;

layout(binding = 0) uniform sampler g_sampler; 
layout(binding = 1) uniform texture2D t_diffuse; 

layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
	vec4 colorTint;
} ubo;

void main()
{
	outcolor = texture(sampler2D(t_diffuse, g_sampler), inUV) * ubo.colorTint;
	outnormal = vec4(inNormal, 1);
}

