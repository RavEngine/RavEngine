struct ParticleVertexOut{
    vec4 position;
};


struct ParticleMatrices{
    mat4 viewProj;
    mat3 billboard;
};

#include "particle_bindings_preamble.glsl"

#include "%s"

#include "particle_bindings_sub.glsl"


layout(location = 0) in vec2 in_position;

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
