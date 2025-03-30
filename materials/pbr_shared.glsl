layout(binding = 0) uniform sampler g_sampler; 
layout(binding = 1) uniform texture2D t_diffuse; 
layout(binding = 2) uniform texture2D t_normal; 
layout(binding = 3) uniform texture2D t_specular; 
layout(binding = 4) uniform texture2D t_metallic; 
layout(binding = 5) uniform texture2D t_roughness; 
layout(binding = 6) uniform texture2D t_ao;
layout(binding = 7) uniform texture2D t_emissive;
#if RVE_HAS_LIGHTMAP_UV
layout(binding = 8) uniform texture2D t_bakedShadow;
#endif

layout(location = 0) in vec2 inUV;
#if RVE_HAS_LIGHTMAP_UV
layout(location = 1) in vec2 inLightmapUV;
#endif

LitOutput frag(EnvironmentData envData)
{
	LitOutput mat_out;

	mat_out.color = texture(sampler2D(t_diffuse, g_sampler), inUV) * ubo.colorTint;
	vec3 normal = texture(sampler2D(t_normal, g_sampler), inUV).rgb;
	normal = normal * 2.0 - 1.0;

	mat_out.normal = normal;

	float specular = texture(sampler2D(t_specular, g_sampler), inUV).r;
	float metallic = texture(sampler2D(t_metallic, g_sampler), inUV).r;
	float roughness = texture(sampler2D(t_roughness, g_sampler), inUV).r;
	mat_out.ao = texture(sampler2D(t_ao, g_sampler), inUV).r;

	mat_out.roughness = roughness * ubo.roughnessTint;
	mat_out.specular = specular * ubo.specularTint;
	mat_out.metallic = metallic * ubo.metallicTint;
	mat_out.emissiveColor = texture(sampler2D(t_emissive, g_sampler), inUV).rgb;

#if RVE_HAS_LIGHTMAP_UV
	// add the baked component
	const vec3 tBakedComponent = texture(sampler2D(t_bakedShadow, g_sampler), inLightmapUV).rgb;

	mat_out.color *= vec4(tBakedComponent,1);
#endif

	return mat_out;
}
