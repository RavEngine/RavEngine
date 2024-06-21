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


void main(){

    uint particle = aliveParticleIndexBuffer[gl_InstanceID];

#if CUSTOM_INDEXING
    ParticleVertexOut user_out = vert(particle, matrixData[0]);
#else
    ParticleVertexOut user_out = vert(particleData[particle], matrixData[0]);
#endif

    gl_Position = user_out.position;

}