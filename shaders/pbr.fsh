#include "ravengine_fsh.h"

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec3[3] inTBN;

FS_OUTPUTS()

layout(binding = 0) uniform sampler g_sampler; 
layout(binding = 1) uniform texture2D t_diffuse; 
layout(binding = 2) uniform texture2D t_normal; 
layout(binding = 3) uniform texture2D t_specular; 
layout(binding = 4) uniform texture2D t_metallic; 
layout(binding = 5) uniform texture2D t_roughness; 
layout(binding = 6) uniform texture2D t_ao;

layout(push_constant, std430) uniform UniformBufferObject{
    mat4 viewProj;
	vec4 colorTint;
	float metallicTint;
	float roughnessTint;
	float specularTint;
} ubo;

void main()
{
	outcolor = texture(sampler2D(t_diffuse, g_sampler), inUV) * ubo.colorTint;
	vec3 normal = texture(sampler2D(t_normal, g_sampler), inUV).rgb;
	normal = normal * 2.0 - 1.0;

	mat3 TBN = mat3(inTBN[0],inTBN[1],inTBN[2]);
	outnormal = vec4(normalize(TBN * normal),1);

	float specular = texture(sampler2D(t_specular, g_sampler), inUV).r;
	float metallic = texture(sampler2D(t_metallic, g_sampler), inUV).r;
	float roughness = texture(sampler2D(t_roughness, g_sampler), inUV).r;
	float ao = texture(sampler2D(t_ao, g_sampler), inUV).r;

	outRoughnessSpecularMetalicAO = vec4(roughness * ubo.roughnessTint, specular * ubo.specularTint,metallic * ubo.metallicTint,ao);
}

