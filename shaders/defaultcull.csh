
layout(push_constant) uniform UniformBufferObject{
	mat4 viewProj;
	uint currentDrawCall;
} ubo;

layout(std430, binding = 0) readonly buffer idBuffer
{
	uint entityIDs[];
};

layout(std430, binding = 1) readonly buffer modelMatrixBuffer
{
	mat4 modelBuffer[];
};

layout(std430, binding = 2) buffer idOutputBuffer
{
	uint entityIDsToRender[];
};


struct IndirectCommand {
	uint indexCount;
	uint instanceCount;
	uint indexStart;
	uint baseVertex;
	uint baseInstance;
};

layout(std430, binding = 3) buffer drawcallBuffer
{
	IndirectCommand commands[];
};

layout(std430, binding = 4) buffer atomicMemoryBuffer
{
	uint atomicMemoryData[];
};

layout(binding = 4) uniform atomic_uint instanceCountAtomic;
layout(binding = 4) uniform atomic_uint baseInstanceAtomic;

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {

	// reset atomics
	if (gl_GlobalInvocationID.x == 0) {
		atomicCounterExchange(instanceCountAtomic, 0);
		atomicCounterExchange(baseInstanceAtomic, 0);
	}

	const uint entityID = entityIDs[gl_GlobalInvocationID.x];
	mat4 model = modelBuffer[entityID];

	// check 1: am I on camera?
	bool isOnCamera = true;

	// check 2: am I in this LOD?
	bool isInThisLod = true;

	// if both checks are true, atomic-increment the instance count and write the entity ID into the output ID buffer based on the previous value of the instance count
	if (isOnCamera && isInThisLod) {
		uint idx = atomicCounterIncrement(instanceCountAtomic);
		entityIDsToRender[idx] = entityID;
	}

	// copy atomics
	if (gl_GlobalInvocationID.x == 0) {
		commands[ubo.currentDrawCall].instanceCount = atomicCounter(instanceCountAtomic);
		commands[ubo.currentDrawCall].baseInstance = atomicCounter(baseInstanceAtomic);
	}
}