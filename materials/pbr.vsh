
layout(push_constant, std430) uniform UniformBufferObject{
    mat4 viewProj;
	vec4 colorTint;
	float metallicTint;
	float roughnessTint;
	float specularTint;
} ubo;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec3[3] outTBN;

LitVertexOut vert(EntityIn entity)
{
    mat4 inModel = entity.modelMtx;
	LitVertexOut v_out;

	vec4 worldPos = inModel * vec4(inPosition,1);

	outUV = inUV;

	v_out.position = ubo.viewProj * worldPos;

	vec3 T = normalize(vec3(inModel * vec4(inTangent,   0.0)));
   	vec3 B = normalize(vec3(inModel * vec4(inBitangent, 0.0)));
   	vec3 N = normalize(vec3(inModel * vec4(inNormal,    0.0)));

	outTBN[0] = T;
	outTBN[1] = B;
	outTBN[2] = N;

	return v_out;
	
}
