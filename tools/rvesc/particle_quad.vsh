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
    
    vec2 inPos = quadPositions[gl_VertexIndex];

    uint particle = aliveParticleIndexBuffer[gl_InstanceIndex];

#if CUSTOM_INDEXING
    ParticleVertexOut user_out = vert(particle, matrixData[0], inPos);
#else
    ParticleVertexOut user_out = vert(particleData[particle], matrixData[0], inPos);
#endif

    vec4 worldPos = vec4(user_out.localPosition, 1);
    gl_Position = matrixData[0].viewProj * worldPos;
    clipSpaceZ = gl_Position.z;

    worldPosition = worldPos.xyz;
    viewPosition = (engineConstants[0].viewOnly * vec4(worldPosition,1)).xyz;
    varyingEntityID = emitterState[0].emitterOwnerID;
}
