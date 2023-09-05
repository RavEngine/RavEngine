
layout(location = 0) in vec4 colorintensity;
layout(location = 1) in vec4 positionradius;
layout(location = 2) in float penumbra;
layout(location = 3) in vec3 forward;
layout(location = 4) in flat vec4[4] invViewProj_elts; 

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


struct SpotLightExtraConstants{
    mat4 lightViewProj;
};

layout(scalar, binding = 8) readonly buffer pushConstantSpill
{
	SpotLightExtraConstants constants[];
};


void main()
{
	#include "lighting_preamble.glsl"

	vec3 toLight = normalize(positionradius.xyz - sampledPos.xyz);

	float dist = distance(sampledPos.xyz, positionradius.xyz);

    vec3 result = CalculateLightRadiance(normal, ubo.camPos, sampledPos.xyz, albedo, metallic, roughness, toLight, 1.0 / (dist * dist), colorintensity.xyz * colorintensity.w);

    float pcfFactor = 1;
	if (bool(ubo.isRenderingShadows)){
		pcfFactor = pcfForShadow(sampledPos, constants[0].lightViewProj, shadowSampler, t_depthshadow);
	}

	// is it inside the light?
	float coneDotFactor = positionradius.w;
	float pixelAngle = dot(-forward,toLight);
	pcfFactor = pcfFactor * (int(pixelAngle > coneDotFactor));

	outcolor = vec4(result * pcfFactor * ao, 1);

	}
