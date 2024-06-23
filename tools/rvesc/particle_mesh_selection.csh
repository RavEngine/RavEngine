
#include "%s"

struct ParticleState
{
    uint aliveParticleCount;
    uint freeListCount;
    uint createdThisFrame;
    uint emitterOwnerID;
};

struct EngineConfig{
    uint numMeshes;
    uint totalPossibleParticles;
};

struct IndirectIndexedCommand {
	uint indexCount;
    uint instanceCount;
    uint indexStart;
    uint baseVertex;
    uint baseInstance;  
};

layout(std430, binding = 10) buffer meshIndexSSBO
{   
    uint meshIndexBuffer[];
};

layout(std430, binding = 11) buffer indirectSSBO
{   
    IndirectIndexedCommand drawCommands[];
};

layout(std430, binding = 12) readonly buffer EngineConfigSSBO
{   
    EngineConfig config[];
};

layout(std430, binding = 13) readonly buffer particleStateSSBO
{   
    ParticleState emitterState[];
};

layout(std430, binding = 14) readonly buffer aliveSSBO
{
    uint aliveParticleIndexBuffer[];
};

layout(scalar, binding = 15) readonly buffer particleDataSSBO
{
    ParticleData particleData[];
};



layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main(){

    if (gl_GlobalInvocationID.x > emitterState[0].aliveParticleCount){
        return;
    }

    uint particleID = aliveParticleIndexBuffer[gl_GlobalInvocationID.x];

    ParticleData particleData = particleData[particleID];

    // get the mesh for this particle from the user
    uint meshID = choose_mesh(particleData, particleID);

    // increment the count for the mesh
    uint localOffset = atomicAdd(drawCommands[meshID].instanceCount, 1);

    // write the particle ID to the appropriate spot in the index buffer
    meshIndexBuffer[config[0].totalPossibleParticles * meshID + localOffset] = particleID;
}