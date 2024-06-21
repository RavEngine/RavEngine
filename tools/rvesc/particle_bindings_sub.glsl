
#if !CUSTOM_INDEXING
layout(scalar, binding = 12) readonly buffer ParticleDataSSBO
{
    ParticleData particleData[];
};
#endif

layout(std430, binding = 13) readonly buffer aliveSSBO
{
    uint aliveParticleIndexBuffer[];
};

layout(scalar, binding = 14) readonly buffer matrixSSBO
{
    ParticleMatrices matrixData[];
};

