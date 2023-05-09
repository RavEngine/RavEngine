#version 450

layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
	vec4 colorTint;
} ubo;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 3) in vec4 inModelR1;
layout(location = 4) in vec4 inModelR2;
layout(location = 5) in vec4 inModelR3;
layout(location = 6) in vec4 inModelR4;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outUV;

void main()
{
	mat4 inModel = mat4(
		inModelR1,
		inModelR2,
		inModelR3,
		inModelR4
	);

	vec4 worldPos = inModel * vec4(inPosition,1);
	outNormal = normalize(transpose(mat3(inModel)) * inNormal);

	outUV = inUV;

	gl_Position = ubo.viewProj * worldPos;
}
