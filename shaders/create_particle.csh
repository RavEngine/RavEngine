#extension GL_EXT_debug_printf : enable

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
    uint createdThisFrame;
};

layout(std430, binding = 2) buffer particleStateSSBO
{   
    ParticleState particleState[];
};

layout(std430, binding = 3)buffer createdThisFrameSSBO
{
    uint particlesCreatedThisFrameBuffer[];
};


layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main()
{
    // only this many particles can be spawned, don't try to spawn more
    if (gl_GlobalInvocationID.x >= ubo.particlesToSpawn){
        return;
    }

    // is another particle possible?
    const uint particleBufferSlot = atomicAdd(particleState[0].aliveParticleCount,1);
    if (particleBufferSlot >= ubo.maxTotalParticles){
        atomicAdd(particleState[0].aliveParticleCount,-1);
        return;
    }

    atomicAdd(particleState[0].createdThisFrame,1);    
   
    // first try getting from the freelist
    // the lower order threads grab from the free list, while the higher order threads create new particles (if applicable)
    int freelistSize = atomicAdd(particleState[0].freeListCount,-1);
    bool canGetFromFreelist = freelistSize > 0;
    
    //debugPrintfEXT("freelistSize = %d", freelistSize);

    uint particleID = 0;
    if (canGetFromFreelist){
         // get it from the freelist
        particleID = particleFreelist[freelistSize - 1];
    }
    else{
        // if we can't get it from the freelist, then create a new one if possible.
        particleID = particleBufferSlot;
        atomicAdd(particleState[0].freeListCount,1);
    }

    // set the particle
    aliveParticleIndexBuffer[particleBufferSlot] = particleID;
    const uint createdThisFrameIdx = gl_GlobalInvocationID.x;
    particlesCreatedThisFrameBuffer[createdThisFrameIdx] = particleID;

}