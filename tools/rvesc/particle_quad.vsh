
struct ParticleVertexOut{
    vec4 position;
};

layout(location = 0) in vec2 in_position;

#include "%s"

layout(scalar, binding = 0) readonly buffer ParticleDataSSBO
{
    ParticleData particleData[];
};

layout(std430, binding = 1) readonly buffer aliveSSBO
{
    uint aliveParticleIndexBuffer[];
};


void main(){

    uint particle = aliveParticleIndexBuffer[gl_InstanceID];

    ParticleVertexOut user_out = vertex(particleData[particle]);

    gl_Position = user_out.position;
}