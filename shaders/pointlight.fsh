
layout(location = 0) in flat vec3 lightColor;
layout(location = 1) in flat float radius;
layout(location = 2) in flat vec3 inPosition;
layout(location = 3) in flat float intensity;

layout(binding = 0) uniform sampler2D s_albedo;
layout(binding = 1) uniform sampler2D s_normal;
layout(binding = 2) uniform sampler2D s_depth;

layout(location = 0) out vec4 outcolor;

layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
	mat4 invViewProj;
    ivec4 viewRect;
} ubo;

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
	
	vec3 albedo = texture(s_albedo, texcoord).xyz;
	vec3 normal = texture(s_normal, texcoord).xyz;
	float depth = texture(s_depth, texcoord).x;
	vec3 pos = ComputeWorldSpacePos(texcoord,depth,ubo.invViewProj);
	
	vec3 toLight = normalize(inPosition - pos);
	
	float dst = distance(pos,inPosition);
	
	int falloffpower = 2;	//1 for linear, 2 for quadratic, 3 for cubic, ...
	
	float attenuation = pow(max(radius-dst,0),falloffpower) * (1.0/pow(radius,falloffpower));
	
	float nDotL = max(dot(normal, toLight), 0);
	
	vec3 diffuseLight = albedo * nDotL;
		
	outcolor = vec4(intensity * attenuation * lightColor * diffuseLight * enabled, enabled);
}
