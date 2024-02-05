
layout(location = 0) in flat vec3 lightColor;
layout(location = 1) in flat float radius;
layout(location = 2) in flat vec3 inPosition;
layout(location = 3) in flat float intensity;
layout(location = 4) in flat vec4[4] invViewProj_elts; 

#define SHADOW_TEX_T textureCube
#include "lightingbindings_shared.h"
#include "ravengine_shader.glsl"
#include "BRDF.glsl"

layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
    ivec4 viewRect;
	ivec4 viewRegion;   // for the virtual screen
	vec3 camPos;
    int isRenderingShadows;
} ubo;

struct PointLightExtraConstants{
    mat4 lightProj;
};

layout(scalar, binding = 8) readonly buffer pushConstantSpill
{
	PointLightExtraConstants constants[];
};

void main()
{
	#include "lighting_preamble.glsl"

	vec3 toLight = normalize(inPosition - sampledPos.xyz);

	float dist = distance(sampledPos.xyz, inPosition);

    vec3 result = CalculateLightRadiance(normal, ubo.camPos, sampledPos.xyz, albedo, metallic, roughness, toLight, 1.0 / (dist * dist), lightColor * intensity);

	float pcfFactor = 1;
	if (bool(ubo.isRenderingShadows)){

		// adapted from: https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/Mesh.hlsl#L233
		vec3 shadowPos = sampledPos.xyz - inPosition;
		float shadowDistance = length(shadowPos);
		vec3 shadowDir = normalize(shadowPos);

		float projectedDistance  = max(max(abs(shadowPos.x), abs(shadowPos.y)), abs(shadowPos.z));

		float nearClip = 0.01;
   		float a = 0.0;
    	float b = nearClip;
    	float z = projectedDistance * a + b;
    	float dbDistance = z / projectedDistance;

		pcfFactor = texture(samplerCubeShadow(t_depthshadow, shadowSampler), vec4(shadowDir, dbDistance)).r;
	}

	outcolor = vec4(result * pcfFactor * ao, 1);
}
