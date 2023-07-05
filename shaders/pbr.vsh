#version 450

layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
	vec4 colorTint;
} ubo;


layout(std430, binding = 2) readonly buffer modelMatrixBuffer
{
	mat4 model[];
};


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

// per-instance
layout(location = 3) in uint inEntityID;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outUV;

void main()
{
	mat4 inModel = model[inEntityID];

	vec4 worldPos = inModel * vec4(inPosition,1);
	outNormal = normalize(mat3(inModel) * inNormal);

	outUV = inUV;

	gl_Position = ubo.viewProj * worldPos;
}
