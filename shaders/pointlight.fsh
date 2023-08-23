
layout(location = 0) in flat vec3 lightColor;
layout(location = 1) in flat float radius;
layout(location = 2) in flat vec3 inPosition;
layout(location = 3) in flat float intensity;
layout(location = 4) in flat vec4[4] invViewProj_elts; 

#include "lightingbindings_shared.h"
#include "ravengine_shader.glsl"

layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
    ivec4 viewRect;
	ivec4 viewRegion;   // for the virtual screen
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
	//calculate sampling positions using fragment pos and view dimensions
	vec2 texcoord = vec2(gl_FragCoord.x / ubo.viewRect[2], gl_FragCoord.y / ubo.viewRect[3]);
    
    // is this pixel visible to the light? if not, discard
	int enabled = 1;
	
	 mat4 invViewProj = mat4(invViewProj_elts[0],invViewProj_elts[1],invViewProj_elts[2],invViewProj_elts[3]);
	vec3 albedo = texture(sampler2D(t_albedo, g_sampler), texcoord).xyz;
	vec3 normal = texture(sampler2D(t_normal, g_sampler), texcoord).xyz;
	float depth = texture(sampler2D(t_depth, g_sampler), texcoord).x;

	vec2 viewTexcoord = (gl_FragCoord.xy - ubo.viewRegion.xy) / ubo.viewRegion.zw;
	vec3 pos = ComputeWorldSpacePos(viewTexcoord,depth, invViewProj);
	
	vec3 toLight = normalize(inPosition - pos);
	
	float dst = distance(pos,inPosition);
	
	int falloffpower = 2;	//1 for linear, 2 for quadratic, 3 for cubic, ...
	
	float attenuation = pow(max(radius-dst,0),falloffpower) * (1.0/pow(radius,falloffpower));
	
	float nDotL = max(dot(normal, toLight), 0);
	
	vec3 diffuseLight = albedo * nDotL;
		
	outcolor = vec4(intensity * attenuation * lightColor * diffuseLight * enabled, enabled);
}
