
layout(push_constant, std430) uniform UniformBufferObject{
    mat4 viewProj;
	vec4 colorTint;
	float metallicTint;
	float roughnessTint;
	float specularTint;
} ubo;


LitVertexOut vertex(mat4 inModel)
{
	LitVertexOut v_out;

	vec4 worldPos = inModel * vec4(inPosition,1);

	v_out.uv = inUV;

	v_out.position = ubo.viewProj * worldPos;

	v_out.T = normalize(vec3(inModel * vec4(inTangent,   0.0)));
   	v_out.B = normalize(vec3(inModel * vec4(inBitangent, 0.0)));
   	v_out.N = normalize(vec3(inModel * vec4(inNormal,    0.0)));

	return v_out;
	
}
