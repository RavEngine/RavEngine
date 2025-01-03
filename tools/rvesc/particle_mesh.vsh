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

#define VARYINGDIR out
#include "mesh_varyings.glsl"

void main(){

// if we are in single-mesh mode, then the particle ID can be found at buffer[gl_InstanceID].
// if we are in multi-mesh mode, then the particle ID can be found at (gl_BaseInstance * config.maxPossibleParticles) + gl_InstanceID
    // we only need to do the second line because if in single-mesh mode, gl_BaseInstance will always be 0

    const uint realBaseInstance = gl_InstanceIndex - gl_BaseInstance;

    uint particle = aliveParticleIndexBuffer[gl_BaseInstance * config[0].maxPossibleParticles + realBaseInstance];

#if CUSTOM_INDEXING
    ParticleVertexOut user_out = vert(particle, matrixData[0]);
#else
    ParticleVertexOut user_out = vert(particleData[particle], matrixData[0]);
#endif

    vec4 worldPos = vec4(user_out.localPosition, 1);

    inTBN[0] = inTangent;
	inTBN[1] = inBitangent;
	inTBN[2] = inNormal;

    gl_Position = matrixData[0].viewProj * worldPos;
    clipSpaceZ = gl_Position.z;
    worldPosition = worldPos.xyz;
    viewPosition = (engineConstants[0].viewOnly * vec4(worldPosition,1)).xyz;
    varyingEntityID = emitterState[0].emitterOwnerID;
}
