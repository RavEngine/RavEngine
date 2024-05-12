
layout(push_constant, std430) uniform UniformBufferObject{
    uint particlesToSpawn;
    uint maxTotalParticles;
} ubo;

layout(std430, binding = 0) buffer aliveSSBO
{
    uint aliveParticleIndexBuffer[];
};

layout(std430, binding = 1)buffer freelistSSBO
{
    uint particleFreelist[];
};

struct ParticleState
{
    uint aliveParticleCount;
    uint freeListCount;
};

layout(std430, binding = 2) buffer particleStateSSBO
{   
    ParticleState particleState[];
};


layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main()
{
    
}