#include "ravengine_fsh.h"

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec3[3] inTBN;

FS_OUTPUTS()

layout(binding = 0) uniform sampler g_sampler; 
layout(binding = 1) uniform texture2D t_diffuse; 
layout(binding = 2) uniform texture2D t_normal; 

layout(push_constant, std430) uniform UniformBufferObject{
    mat4 viewProj;
	vec4 colorTint;
	vec3 metallicTint;
	vec3 roughnessTint;
	vec3 specularTint;
} ubo;

void main()
{
	outcolor = texture(sampler2D(t_diffuse, g_sampler), inUV) * ubo.colorTint;
	vec3 normal = texture(sampler2D(t_normal, g_sampler), inUV).rgb;
	normal = normal * 2.0 - 1.0;

	mat3 TBN = mat3(inTBN[0],inTBN[1],inTBN[2]);
	outnormal = vec4(normalize(TBN * normal),1);

	outspecular = vec4(ubo.specularTint,1);
	outroughness = vec4(ubo.roughnessTint,1);
	outmetallic = vec4(ubo.metallicTint,1);
}

