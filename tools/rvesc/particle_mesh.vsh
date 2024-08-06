#include "particle_shared.glsl"

struct ParticleMatrices{
    mat4 viewProj;
};

#include "lit_mesh_bindings.glsl"

#include "particle_bindings_preamble.glsl"

#include "%s"

#include "particle_bindings_sub.glsl"

#include "lit_mesh_shared.glsl"

struct EngineData2{
    uint numMeshes;
    uint maxPossibleParticles;
};

layout(scalar, binding = 21) readonly buffer engineDataSSBO
{
    EngineData2 config[];
};

layout(location = 11) out vec3 worldPosition;
layout(location = 12) out vec3 viewPosition;

void main(){

// if we are in single-mesh mode, then the particle ID can be found at buffer[gl_InstanceID].
// if we are in multi-mesh mode, then the particle ID can be found at (gl_BaseInstance * config.maxPossibleParticles) + gl_InstanceID
    // we only need to do the second line because if in single-mesh mode, gl_BaseInstance will always be 0

    const uint realBaseInstance = gl_InstanceID - gl_BaseInstance;

    uint particle = aliveParticleIndexBuffer[gl_BaseInstance * config[0].maxPossibleParticles + realBaseInstance];

#if CUSTOM_INDEXING
    ParticleVertexOut user_out = vert(particle, matrixData[0]);
#else
    ParticleVertexOut user_out = vert(particleData[particle], matrixData[0]);
#endif

    gl_Position = user_out.position;
    worldPosition = user_out.worldPosition;
    viewPosition = (engineConstants[0].viewOnly * vec4(worldPosition,1)).xyz;
}