
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

struct IndirectWorkgroupSize {
    uint x;
    uint y;
    uint z;
};

layout(std430, binding = 1)buffer indirectSSBO
{
    IndirectWorkgroupSize indirectBuffers[];    // 0 is initialization shader, 1 is update shader
};

// kinda sucks...
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main(){

    if (gl_GlobalInvocationID.x > 0){
        return;
    }

    indirectBuffers[0] = IndirectWorkgroupSize(uint(ceil(particleState[0].createdThisFrame/64.f)),1,1);  // initialization shader
    indirectBuffers[1] = IndirectWorkgroupSize(uint(ceil(particleState[0].aliveParticleCount/64.f)),1,1);  // update shader

}