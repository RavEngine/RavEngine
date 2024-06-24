struct ParticleVertexOut{
    vec4 position;
};

struct ParticleMatrices{
    mat4 viewProj;
};

#include "lit_mesh_bindings.glsl"

#include "particle_bindings_preamble.glsl"

#include "%s"

#include "particle_bindings_sub.glsl"

struct EngineData{
    uint numMeshes;
    uint maxPossibleParticles;
};

layout(scalar, binding = 11) readonly buffer engineDataSSBO
{
    EngineData config[];
};


void main(){

// if we are in single-mesh mode, then the particle ID can be found at buffer[gl_InstanceID].
// if we are in multi-mesh mode, then the particle ID can be found at (gl_BaseInstance * config.maxPossibleParticles) + gl_InstanceID
    // we only need to do the second line because if in single-mesh mode, gl_BaseInstance will always be 0

    uint particle = aliveParticleIndexBuffer[gl_BaseInstance * config[0].maxPossibleParticles + gl_InstanceID];

#if CUSTOM_INDEXING
    ParticleVertexOut user_out = vert(particle, matrixData[0]);
#else
    ParticleVertexOut user_out = vert(particleData[particle], matrixData[0]);
#endif

    gl_Position = user_out.position;

}