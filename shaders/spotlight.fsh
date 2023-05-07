layout(location = 0) in vec4 colorintensity;
layout(location = 1) in vec4 positionradius;
layout(location = 2) in float penumbra;
layout(location = 3) in vec3 forward;

layout(binding = 0) uniform sampler2D s_albedo;
layout(binding = 1) uniform sampler2D s_normal;
layout(binding = 2) uniform sampler2D s_depth;

layout(location = 0) out vec4 outcolor;

layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
	mat4 invViewProj;
    ivec4 viewRect;
} ubo;

float remap(float value, float min1, float max1, float min2, float max2) {
  return clamp(min2 + (value - min1) * (max2 - min2) / (max1 - min1),min2,max2);
}

void main()
{
	//calculate sampling positions using fragment pos and view dimensions
	vec2 texcoord = vec2(gl_FragCoord.x / ubo.viewRect[2], gl_FragCoord.y / ubo.viewRect[3]);

	 // is this pixel visible to the light? if not, discard
	int enabled = 1;

	
	float intensity = colorintensity[3];
	float coneDotFactor = positionradius.w;
	
	vec3 albedo = texture(s_albedo, texcoord).xyz;
	vec3 normal = texture(s_normal, texcoord).xyz;
	float depth = texture(s_depth, texcoord).x;
	vec3 pos =  (ubo.invViewProj * vec4(gl_FragCoord.x,gl_FragCoord.y,depth,1)).xyz;
	
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
