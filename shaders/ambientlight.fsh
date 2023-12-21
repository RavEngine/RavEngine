
layout(location = 0) in vec4 colorintensity;
layout(location = 0) out vec4 outcolor;

layout(push_constant) uniform UniformBufferObject{
	ivec4 viewRect;
	uint ssao_enabled;
} ubo;

layout(binding = 0) uniform sampler g_sampler;
layout(binding = 1) uniform texture2D t_albedo;
layout(binding = 2) uniform texture2D t_ssao;

layout(early_fragment_tests) in;
void main()
{
	//calculate sampling positions using fragment pos and view dimensions
	vec2 texcoord = vec2(gl_FragCoord.x / ubo.viewRect[2], gl_FragCoord.y / ubo.viewRect[3]);
	
	vec4 lightColor = colorintensity;
	
	float intensity = lightColor[3];
	
	vec3 albedo = texture(sampler2D(t_albedo,g_sampler), texcoord).xyz;

	float ao = 1;
	if (ubo.ssao_enabled > 0){
		ao = texture(sampler2D(t_ssao, g_sampler), texcoord).r;
	}
	
	outcolor = vec4(albedo * intensity * lightColor.xyz * ao, 1);
}
