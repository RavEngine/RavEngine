
struct VertexNormalUV {
	vec3 pos;
	vec2 uv;
	vec3 normal;
};

layout(std430, binding = 0) buffer matrixOutputMatrixBuffer
{
	VertexNormalUV vertexOutput[];
};

layout(std430, binding = 1) readonly buffer vertexInputBuffer
{
	VertexNormalUV inputVerts[];
};


layout(std430, binding = 2) readonly buffer poseMatrixBuffer
{
    mat4 pose[];
};

struct weight{
	uint joint_index;
	float influence;
};

layout(std430, binding = 3) readonly buffer weightsBuffer
{
    weight weights[];				// index, influence
};


layout(push_constant) uniform UniformBufferObject{
	uint numObjects;
	uint numVertices;
	uint numBones;
	uint boneReadOffset;
	uint vertexWriteOffset;	
	uint vertexReadOffset;
} ubo;

#define NUM_INFLUENCES 4

layout (local_size_x = 8, local_size_y = 32, local_size_z = 1) in;	//x = object #, y = vertex #
void main()
{
	//prevent out-of-bounds
	if (gl_GlobalInvocationID.y < ubo.numVertices && gl_GlobalInvocationID.x < ubo.numObjects){
		// for readability
		const uint NumObjects = ubo.numObjects;
		const uint numVerts = ubo.numVertices;
		const uint numBones = ubo.numBones;
		const uint vertID = gl_GlobalInvocationID.y;
		const uint objID = gl_GlobalInvocationID.x;
		
		const uint weightsid = vertID;		//1x vec4 elements elements per vertex, is always the same per vertex
		
		const uint bone_begin = numBones * objID + ubo.boneReadOffset; //offset to the bone for the correct object
				
		//will become the pose matrix
		mat4 totalmtx = mat4(vec4(0,0,0,0),vec4(0,0,0,0),vec4(0,0,0,0),vec4(0,0,0,0));
		
		for(uint i = 0; i < NUM_INFLUENCES; i++){
			const uint idx = weightsid + i;
			const weight weightdataBuffer = weights[idx];

			uint joint_idx = weightdataBuffer.joint_index;
			float weight = weightdataBuffer.influence;

			//get the pose and bindpose of the target joint
			mat4 posed_mtx = pose[bone_begin + joint_idx];
			totalmtx += weight * posed_mtx;	
		}
		
		//destination to write the matrix
		const uint writeOffset = (vertID + objID * numVerts) + ubo.vertexWriteOffset;	//1x mat4 elements per object
	
		const uint readOffset = (vertID) + ubo.vertexReadOffset;	// there is only one copy of the vertex read data because it comes from the unified mesh data buffer

		VertexNormalUV readVertex = inputVerts[readOffset];
		readVertex.pos = (totalmtx * vec4(readVertex.pos,1)).xyz;
		readVertex.normal = mat3(totalmtx) * readVertex.normal;

		//write matrix
		vertexOutput[writeOffset] = readVertex;
	}
}
