
layout(location = 0) in vec3 lightdir;
layout(location = 1) in vec4 colorintensity;
layout(location = 2) in flat vec4[4] invViewProj_elts; 


#include "lightingbindings_shared.h"
#include "ravengine_shader.glsl"
#include "BRDF.glsl"

struct DirLightExtraConstants{
    mat4 lightViewProj;
};

layout(scalar, binding = 8) readonly buffer pushConstantSpill
{
	DirLightExtraConstants constants[];
};

layout(push_constant, std430) uniform UniformBufferObject{
    mat4 viewProj;
    ivec4 viewRect;     // for the whole window
    ivec4 viewRegion;   // for the virtual screen
    vec3 camPos;
    int isRenderingShadows;
} ubo;



void main()
{
	#include "lighting_preamble.glsl"

    vec3 toLight = normalize(lightdir.xyz);

    vec3 result = CalculateLightRadiance(normal, ubo.camPos, sampledPos.xyz, albedo, metallic, roughness, toLight, 1, colorintensity.xyz * colorintensity.w);

    // is this pixel visible to the light?
    float pcfFactor = 1;
    if (bool(ubo.isRenderingShadows)){
        pcfFactor = pcfForShadow(sampledPos, constants[0].lightViewProj, shadowSampler, t_depthshadow);
    }
    
    outcolor = vec4(result * pcfFactor, 1);
	
}
