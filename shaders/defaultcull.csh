#extension GL_EXT_scalar_block_layout : enable

layout(push_constant) uniform UniformBufferObject{
	mat4 viewProj;
	vec3 bbmin;
	uint indirectBufferOffset;			// needs to be like this because of padding / alignment
	vec3 bbmax;
	uint numObjects;
	uint cullingBufferOffset;
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

bool pointIsOnCamera(vec3 point){
	return 
		point.x >= -1 && point.x <= 1
		&& point.y >= -1 && point.y <= 1
		&& point.z >= -1 && point.z <= 1;
}

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
	// bail
	const uint currentEntity = gl_GlobalInvocationID.x;
	if (currentEntity >= ubo.numObjects) {
		return;
	}

	const uint entityID = entityIDs[currentEntity];
	mat4 model = modelBuffer[entityID];

	vec3 bbmin = ubo.bbmin;
	vec3 bbmax = ubo.bbmax;

	// check 1: am I on camera?
	const vec3 bbox[8] = {
		bbmin,							// 'min' side
		vec3(-bbmin.x, bbmin.y, bbmin.z),
		vec3(-bbmin.x, -bbmin.y, bbmin.z),
		vec3(bbmin.x, bbmin.y, -bbmin.z),

		bbmax,							// 'max' size
		vec3(-bbmax.x, bbmax.y, bbmax.z),
		vec3(-bbmax.x, -bbmax.y, bbmax.z),
		vec3(bbmax.x, bbmax.y, -bbmax.z),
	};

	bool isOnCamera = false;
	// is considered on camera if any point in the bounding box is on camera
	for(uint i = 0; i < 6; i++){
		vec4 point = vec4(bbox[i],1.0);
		point = model * point;
		point = ubo.viewProj * point;

		isOnCamera = isOnCamera || pointIsOnCamera(point.xyz / point.w);
	}

	// check 2: what LOD am I in
	uint lodID = 0;	//TODO: when multi-LOD support is added

	// if both checks are true, atomic-increment the instance count and write the entity ID into the output ID buffer based on the previous value of the instance count
	if (isOnCamera) {
		uint idx = atomicAdd(indirectBuffer[ubo.indirectBufferOffset + lodID].instanceCount,1);
		uint idxLODOffset = ubo.numObjects * lodID + ubo.cullingBufferOffset;
		entityIDsToRender[idx + idxLODOffset] = entityID;
	}

}