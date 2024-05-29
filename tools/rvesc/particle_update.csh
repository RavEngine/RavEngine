#include "%s"

struct ParticleState
{
    uint aliveParticleCount;
    uint freeListCount;
    uint createdThisFrame;
    uint emitterOwnerID;
};

layout(std430, binding = 0) buffer particleStateSSBO
{   
    ParticleState particleState[];
};

layout(std430, binding = 1) buffer aliveSSBO
{
    uint aliveParticleIndexBuffer[];
};

layout(scalar, binding = 2) buffer particleDataSSBO
{
    ParticleData particleData[];
};


layout(std430, binding = 3) buffer lifeSSBO
{
    float particleLifeBuffer[];
};

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main(){

    if (gl_GlobalInvocationID.x >= particleState[0].aliveParticleCount){
        return;
    }

    // fetch built-in particle data
    uint particleID = aliveParticleIndexBuffer[gl_GlobalInvocationID.x];
    ParticleData data = particleData[particleID];
    float particleLife = particleLifeBuffer[particleID];

    // get changes from the user
    update(data, particleLife, particleID);

    // commit changes
    particleLifeBuffer[particleID] = particleLife;
    particleData[particleID] = data;
}
