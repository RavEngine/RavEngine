
layout(push_constant, std430) uniform UniformBufferObject{
    uint maxTotalParticles;
} ubo;

struct ParticleState
{
    int aliveParticleCount;
    uint freeListCount;
    uint createdThisFrame;
};

layout(std430, binding = 0) buffer particleStateSSBO
{   
    ParticleState particleState[];
};

layout(std430, binding = 1) buffer aliveSSBO
{
    uint aliveParticleIndexBuffer[];
};

layout(std430, binding = 2)buffer freelistSSBO
{
    uint particleFreelist[];
};

layout(std430, binding = 3) buffer lifeSSBO
{
    float particleLifeBuffer[];
};

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main(){
    if (gl_GlobalInvocationID.x == 0){
        // reset # created this frame
        particleState[0].createdThisFrame = 0;
    }

    if (gl_GlobalInvocationID.x >= atomicAdd(particleState[0].aliveParticleCount,0)){
        return;
    }

    // find all particles with life <= 0 and recycle those
    uint particleID = aliveParticleIndexBuffer[gl_GlobalInvocationID.x];
    if (particleLifeBuffer[particleID] > 0){
        return;
    }

    // add the particle to the freelist
    const uint freelistIdx = atomicAdd(particleState[0].freeListCount,1);
    if (freelistIdx >= ubo.maxTotalParticles){
        return;         // safety mesaure that shouldn't ever trigger
    }
    particleFreelist[freelistIdx] = particleID;

    // replace this slot at invocationID.x with the particle at the end of the alive buffer
    int prevTotalAlive = atomicAdd(particleState[0].aliveParticleCount,-1);

    aliveParticleIndexBuffer[gl_GlobalInvocationID.x] = aliveParticleIndexBuffer[prevTotalAlive-1]; // convert to index
}