
layout(push_constant, std430) uniform UniformBufferObject{
    mat4 viewProj;
	vec4 colorTint;
	float metallicTint;
	float roughnessTint;
	float specularTint;
} ubo;

#include "pbr_shared.glsl"