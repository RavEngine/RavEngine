
layout(push_constant, std430) uniform UniformBufferObject{
	vec4 colorTint;
	float metallicTint;
	float roughnessTint;
	float specularTint;
} ubo;

layout(location = 0) out vec2 outUV;
#if RVE_HAS_LIGHTMAP_UV
layout(location = 1) out vec2 outlightmapUV;
#endif

LitVertexOut vert(EntityIn entity, EngineData data)
{
	LitVertexOut v_out;

    v_out.localPosition = inPosition;

	outUV = inUV;
#if RVE_HAS_LIGHTMAP_UV
	outlightmapUV = inLightmapUV;
#endif

	return v_out;
	
}
