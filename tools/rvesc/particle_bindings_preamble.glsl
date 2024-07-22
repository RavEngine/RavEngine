// because GLSL doesn't allow passing buffers to functions for ... reasons
#if CUSTOM_INDEXING
layout(scalar, binding = 18) readonly buffer ParticleDataSSBO
{
    uint particleDataBytes[];
};
#endif