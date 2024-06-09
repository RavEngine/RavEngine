
struct ParticleVertexOut{
    vec4 position;
};

struct ParticleMatrices{
    mat4 viewProj;
    mat3 billboard;
};

#include "%s"

layout(location = 0) in vec2 in_position;

layout(scalar, binding = 0) readonly buffer ParticleDataSSBO
{
    ParticleData particleData[];
};

layout(std430, binding = 1) readonly buffer aliveSSBO
{
    uint aliveParticleIndexBuffer[];
};



layout(std430, binding = 2) readonly buffer matrixSSBO
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

    ParticleVertexOut user_out = vert(particleData[particle], matrixData[0], inPos);

    gl_Position = user_out.position;
}
