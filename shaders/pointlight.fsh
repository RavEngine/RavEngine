
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
    mat4 lightViewProj;
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
		float origLength = length(inPosition - sampledPos.xyz);
		float depth = texture(samplerCube(t_depthshadow,shadowSampler), toLight).r;
		//TODO: convert depth to linear coordinates
		depth *= 100;
		if (origLength > depth){
			pcfFactor = 0;
		}
	}

	outcolor = vec4(result * pcfFactor * ao, 1);
}
