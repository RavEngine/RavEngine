#extension GL_EXT_scalar_block_layout : enable

layout(location = 0) in vec4 colorintensity;
layout(location = 1) in vec4 positionradius;
layout(location = 2) in float penumbra;
layout(location = 3) in vec3 forward;
layout(location = 4) in flat vec4[4] invViewProj_elts; 

layout(binding = 0) uniform sampler g_sampler;
layout(binding = 1) uniform sampler shadowSampler;
layout(binding = 2) uniform texture2D t_albedo;
layout(binding = 3) uniform texture2D t_normal;
layout(binding = 4) uniform texture2D t_depth;
layout(binding = 5) uniform texture2D t_depthshadow;

layout(location = 0) out vec4 outcolor;

layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
    ivec4 viewRect;
    bool isRenderingShadows;
} ubo;


struct SpotLightExtraConstants{
    mat4 lightViewProj;
};

layout(scalar, binding = 8) readonly buffer pushConstantSpill
{
	SpotLightExtraConstants constants[];
};

float remap(float value, float min1, float max1, float min2, float max2) {
  return clamp(min2 + (value - min1) * (max2 - min2) / (max1 - min1),min2,max2);
}

vec4 ComputeClipSpacePosition(vec2 pos, float depth){
	pos.y = 1.0 - pos.y;
	vec4 positionCS = vec4(pos * 2.0 - 1.0, depth, 1);
	return positionCS;
}

vec3 ComputeWorldSpacePos(vec2 positionNDC, float depth, mat4 invViewProj){
	vec4 positionCS = ComputeClipSpacePosition(positionNDC, depth);
	vec4 hpositionWS = invViewProj * positionCS;
	return (hpositionWS / hpositionWS.w).xyz;
}

void main()
{
	//calculate sampling positions using fragment pos and view dimensions
	vec2 texcoord = vec2(gl_FragCoord.x / ubo.viewRect[2], gl_FragCoord.y / ubo.viewRect[3]);

	 // is this pixel visible to the light? if not, discard
	int enabled = 1;

	
	float intensity = colorintensity[3];
	float coneDotFactor = positionradius.w;
	
	vec3 albedo = texture(sampler2D(t_albedo, g_sampler), texcoord).xyz;
	vec3 normal = texture(sampler2D(t_normal, g_sampler), texcoord).xyz;
	float depth = texture(sampler2D(t_depth, g_sampler), texcoord).x;
	 mat4 invViewProj = mat4(invViewProj_elts[0],invViewProj_elts[1],invViewProj_elts[2],invViewProj_elts[3]);
	vec3 pos = ComputeWorldSpacePos(texcoord,depth, invViewProj);

	if (ubo.isRenderingShadows){
		//TODO: shadow sampling
	}
	
	vec3 toLight = normalize(positionradius.xyz - pos);
	
	float dist = distance(pos,positionradius.xyz);
	
	int falloffpower = 2;	//1 for linear, 2 for quadratic, 3 for cubic, ...
	
	float attenuation = 1/pow(dist,falloffpower);
	
	float nDotL = max(dot(normal, toLight), 0);
	
	vec3 diffuseLight = albedo * nDotL;

	// is it inside the light?
	float pixelAngle = dot(-forward,toLight);
	enabled = enabled * (int(pixelAngle > coneDotFactor));

	// x is inner, y is outer

	float epsilon = penumbra - coneDotFactor;
	float penumbraFactor = clamp((pixelAngle - coneDotFactor) / epsilon, 0, 1);
	
	outcolor = vec4(attenuation * colorintensity.xyz * diffuseLight * penumbraFactor * enabled, enabled);
}
