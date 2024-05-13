
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
    int freeListCount;
    uint createdThisFrameCount;
};

layout(std430, binding = 2) buffer particleStateSSBO
{   
    ParticleState particleState[];
};

layout(std430, binding = 3)buffer createdThisFrameSSBO
{
    uint particlesCreatedThisFrameBuffer[];
};

struct IndirectWorkgroupSize {
    uint x;
    uint y;
    uint z;
};

layout(std430, binding = 4)buffer indirectSSBO
{
    IndirectWorkgroupSize indirectBuffers[];    // 0 is initialization shader, 1 is update shader
};



layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main()
{
    // only this many particles can be spawned, don't try to spawn more
    if (gl_GlobalInvocationID.x > ubo.particlesToSpawn){
        return;
    }

    // is another particle possible?
    const uint particleBufferSlot = atomicAdd(particleState[0].aliveParticleCount,1);
    if (particleBufferSlot >= ubo.maxTotalParticles){
        return;
    }

   
     // first try getting from the freelist
    const int freelistIndex = atomicAdd(particleState[0].freeListCount, - 1) - 1;

    uint particleID = 0;
    if (freelistIndex < 0){
        // if we can't get it from the freelist, then create a new one if possible.
        // we know that if we are slot N, and there are no free particles in the free list, 
        // then all IDs from [0, N) are in use. Thus, the next ID = slot
        particleID = particleBufferSlot;
    }
    else{
        // get it from the freelist
        particleID = particleFreelist[freelistIndex];
    }

    // set the particle
    aliveParticleIndexBuffer[particleBufferSlot] = particleID;
    const uint createdThisFrameIdx = atomicAdd(particleState[0].createdThisFrameCount, 1);
    particlesCreatedThisFrameBuffer[createdThisFrameIdx] = particleID;

    barrier();
    if (gl_LocalInvocationID.x == 0){
        // pin to max count
        atomicMin(particleState[0].aliveParticleCount, ubo.maxTotalParticles);
        atomicMax(particleState[0].freeListCount, 0);   // pin to 0
    }

    barrier();

    if (gl_GlobalInvocationID.x == 0){
        indirectBuffers[0] = IndirectWorkgroupSize(uint(ceil(particleState[0].createdThisFrameCount/64.f)),1,1);  // initialization shader
        indirectBuffers[1] = IndirectWorkgroupSize(uint(ceil(particleState[0].aliveParticleCount/64.f)),1,1);  // initialization shader
    }
}