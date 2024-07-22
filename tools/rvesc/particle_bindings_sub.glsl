
#if !CUSTOM_INDEXING
layout(scalar, binding = 18) readonly buffer ParticleDataSSBO
{
    ParticleData particleData[];
};
#endif

layout(std430, binding = 19) readonly buffer aliveSSBO
{
    uint aliveParticleIndexBuffer[];
};

layout(scalar, binding = 20) readonly buffer matrixSSBO
{
    ParticleMatrices matrixData[];
};

