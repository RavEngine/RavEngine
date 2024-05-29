
struct ParticleInitData{
    mat4 emitterModel;
    uint particleID;
};

#include "%s"

struct ParticleState
{
    uint aliveParticleCount;
    uint freeListCount;
    uint createdThisFrame;
    uint emitterOwnerID;
};

layout(std430, binding = 0) readonly buffer particleStateSSBO
{   
    ParticleState particleState[];
};

layout(std430, binding = 1) readonly buffer createdThisFrameSSBO
{
    uint particlesCreatedThisFrameBuffer[];
};

layout(scalar, binding = 2) buffer particleDataSSBO
{
    ParticleData particleData[];
};

layout(std430, binding = 3) buffer lifeSSBO
{
    float particleLifeBuffer[];
};

layout(std430, binding = 4) readonly buffer modelMatrixBuffer{
    mat4 model[];
};

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main(){
    // bounds-check
    if (gl_GlobalInvocationID.x >= particleState[0].createdThisFrame){
        return;
    }

    // get the particle ID
    uint particleID = particlesCreatedThisFrameBuffer[gl_GlobalInvocationID.x];


    ParticleInitData initData;
    initData.emitterModel = model[particleState[0].emitterOwnerID];
    initData.particleID = particleID;

    ParticleData data = init(initData);

    particleData[particleID] = data;

    particleLifeBuffer[particleID] = 1.0;
}