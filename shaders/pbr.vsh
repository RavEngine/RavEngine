#version 450

layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
	vec4 colorTint;
} ubo;


layout(std430, binding = 10) readonly buffer modelMatrixBuffer
{
	mat4 model[];
};


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inUV;

// per-instance
layout(location = 10) in uint inEntityID;

layout(location = 0) out vec2 outUV;
layout(location = 1) out flat vec3[3] outTBN;

void main()
{
	mat4 inModel = model[inEntityID];

	vec4 worldPos = inModel * vec4(inPosition,1);

	outUV = inUV;

	gl_Position = ubo.viewProj * worldPos;

	vec3 T = normalize(vec3(inModel * vec4(inTangent,   0.0)));
   	vec3 B = normalize(vec3(inModel * vec4(inBitangent, 0.0)));
   	vec3 N = normalize(vec3(inModel * vec4(inNormal,    0.0)));
	outTBN[0] = T;
	outTBN[1] = B;
	outTBN[2] = N;
}
