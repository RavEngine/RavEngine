
struct ParticleVertexOut{
    vec4 position;
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

    ParticleVertexOut user_out = vert(particleData[particle], inPos);

    gl_Position = user_out.position;
}
