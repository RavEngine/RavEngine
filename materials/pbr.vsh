
layout(push_constant, std430) uniform UniformBufferObject{
	vec4 colorTint;
	float metallicTint;
	float roughnessTint;
	float specularTint;
} ubo;

layout(location = 0) out vec2 outUV;

LitVertexOut vert(EntityIn entity, EngineData data)
{
    mat4 inModel = entity.modelMtx;
	LitVertexOut v_out;

    v_out.localPosition = inPosition;

	outUV = inUV;

	return v_out;
	
}
