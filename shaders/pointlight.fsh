
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

float getViewSpaceZ(vec3 dirvec){
	float maxMagnitude = 0;
	uint maxIndex = 0;
	for(uint i = 0; i < dirvec.length(); i++){
		float currentMag = abs(dirvec[i]);
		if (currentMag > maxMagnitude){
			maxIndex = i;
			maxMagnitude = currentMag;
		}
	}
	return abs(maxMagnitude);
} 

void main()
{
	#include "lighting_preamble.glsl"

	vec3 toLight = normalize(inPosition - sampledPos.xyz);

	float dist = distance(sampledPos.xyz, inPosition);

    vec3 result = CalculateLightRadiance(normal, ubo.camPos, sampledPos.xyz, albedo, metallic, roughness, toLight, 1.0 / (dist * dist), lightColor * intensity);

	float pcfFactor = 1;
	if (bool(ubo.isRenderingShadows)){

		// the face to sample is that with the largest magnitude
		// that value is also the view-space Z
		float viewZ = getViewSpaceZ(inPosition - sampledPos.xyz);
		vec4 projected = constants[0].lightProj * vec4(viewZ, 0, 0, 1);
		viewZ = projected.x / projected.w;

		pcfFactor = texture(samplerCubeShadow(t_depthshadow, shadowSampler), vec4(toLight, viewZ)).r;
	}

	outcolor = vec4(result * pcfFactor * ao, 1);
}
