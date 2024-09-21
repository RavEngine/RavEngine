#include "particle_shared.glsl"

struct ParticleMatrices{
    mat4 viewProj;
    mat3 billboard;
};

#include "particle_bindings_preamble.glsl"

#include "%s"

#include "particle_bindings_sub.glsl"

#include "lit_mesh_shared.glsl"

layout(location = 0) in vec2 in_position;

#define VARYINGDIR out
#include "mesh_varyings.glsl"

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
    clipSpaceZ = gl_Position.z;

    worldPosition = user_out.worldPosition;
    viewPosition = (engineConstants[0].viewOnly * vec4(worldPosition,1)).xyz;
    varyingEntityID = emitterState[0].emitterOwnerID;
}
