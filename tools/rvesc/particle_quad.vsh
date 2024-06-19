struct ParticleVertexOut{
    vec4 position;
};

struct ParticleMatrices{
    mat4 viewProj;
    mat3 billboard;
};

// because GLSL doesn't allow passing buffers to functions for ... reasons
#if CUSTOM_INDEXING
layout(scalar, binding = 0) readonly buffer ParticleDataSSBO
{
    uint particleDataBytes[];
};
#endif

#include "%s"

layout(location = 0) in vec2 in_position;

#if !CUSTOM_INDEXING
layout(scalar, binding = 0) readonly buffer ParticleDataSSBO
{
    ParticleData particleData[];
};
#endif

layout(std430, binding = 1) readonly buffer aliveSSBO
{
    uint aliveParticleIndexBuffer[];
};

layout(scalar, binding = 2) readonly buffer matrixSSBO
{
    ParticleMatrices matrixData[];
};




void main(){
    
    vec2 quadPositions[] =
    {
        vec2(-1, 1),
        vec2(-1, -1),
        vec2(1, 1),
        vec2(1, -1)
    };
    
    vec2 inPos = quadPositions[gl_VertexID];

    uint particle = aliveParticleIndexBuffer[gl_InstanceID];

#if CUSTOM_INDEXING
    ParticleVertexOut user_out = vert(particle, matrixData[0], inPos);
#else
    ParticleVertexOut user_out = vert(particleData[particle], matrixData[0], inPos);
#endif

    gl_Position = user_out.position;
}
