layout(push_constant, std430) uniform UniformBufferObject{
	vec4 colorTint;
	float metallicTint;
	float roughnessTint;
	float specularTint;
    uint bytesPerParticle;
    uint positionOffset;
    uint scaleOffset;
} ubo;