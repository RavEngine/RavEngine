
layout(std430, binding = 0) buffer matrixOutputMatrixBuffer
{
    mat4 matrixOutput[];
};

layout(std430, binding = 1) readonly buffer poseMatrixBuffer
{
    mat4 pose[];
};

struct weight{
	uint joint_index;
	float influence;
};

layout(std430, binding = 2) readonly buffer weightsBuffer
{
    weight weights[];				// index, influence
};


layout(push_constant) uniform UniformBufferObject{
	ivec4 NumObjects;			// x = num objects, y = num vertices, z = num bones, w = offset into transient buffer
	ivec4 ComputeOffsets;		// x = matrixOutput offset, y = unused, z = unused, w = unused
} ubo;

#define NUM_INFLUENCES 4

layout (local_size_x = 8, local_size_y = 32, local_size_z = 1) in;	//x = object #, y = vertex #
void main()
{
	//prevent out-of-bounds
	if (gl_GlobalInvocationID.y < ubo.NumObjects.y && gl_GlobalInvocationID.x < ubo.NumObjects.x){
		// for readability
		const uint NumObjects = ubo.NumObjects.x;
		const uint numVerts = ubo.NumObjects.y;
		const uint numBones = ubo.NumObjects.z;
		const uint vertID = gl_GlobalInvocationID.y;
		const uint objID = gl_GlobalInvocationID.x;
		
		const uint weightsid = vertID * 2;		//2x vec4 elements elements per vertex, is always the same per vertex
		
		const uint bone_begin = numBones * objID * 4 + ubo.NumObjects.w * 4; //offset to the bone for the correct object
				
		//will become the pose matrix
		mat4 totalmtx = mat4(vec4(0,0,0,0),vec4(0,0,0,0),vec4(0,0,0,0),vec4(0,0,0,0));
		
		for(uint i = 0; i < NUM_INFLUENCES; i++){
			const uint idx = weightsid + i;
			const weight weightdataBuffer = weights[idx];

			uint joint_idx = weightdataBuffer.joint_index;
			float weight = weightdataBuffer.influence;

			//get the pose and bindpose of the target joint
			mat4 posed_mtx = pose[bone_begin + joint_idx * 4];
			totalmtx += weight * posed_mtx;	
		}
		
		//destination to write the matrix
		const uint offset = (vertID * 4 + objID * numVerts * 4) + ubo.ComputeOffsets.x;	//1x mat4 elements per object
	
		//write matrix
		matrixOutput[offset] = totalmtx;
	}
}
