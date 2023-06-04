
layout(push_constant) uniform UniformBufferObject{
	mat4 viewProj;
	uint indirectBufferOffset;
	uint numObjects;
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
	IndirectCommand indirectBuffer[];
};

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
	// bail
	const uint currentEntity = gl_GlobalInvocationID.x;
	if (currentEntity >= ubo.numObjects) {
		return;
	}

	const uint entityID = entityIDs[currentEntity];
	mat4 model = modelBuffer[entityID];

	// check 1: am I on camera?
	bool isOnCamera = true;

	// check 2: what LOD am I in
	uint lodID = 0;	//TODO: when multi-LOD support is added

	// if both checks are true, atomic-increment the instance count and write the entity ID into the output ID buffer based on the previous value of the instance count
	if (isOnCamera) {
		uint idx = atomicAdd(indirectBuffer[ubo.indirectBufferOffset + lodID].instanceCount,1);
		uint idxLODOffset = ubo.numObjects * lodID;
		entityIDsToRender[idx + idxLODOffset] = entityID;
	}

}