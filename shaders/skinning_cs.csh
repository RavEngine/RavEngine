
struct VertexNormalUV {
	float pos_1, pos_2, pos_3;
	float normal_1, normal_2, normal_3;
	float uv_1, uv_2;
};

layout(std140, binding = 0) buffer matrixOutputMatrixBuffer
{
	VertexNormalUV vertexOutput[];
};

layout(std140, binding = 1) readonly buffer vertexInputBuffer
{
	VertexNormalUV inputVerts[];
};


layout(std430, binding = 2) readonly buffer poseMatrixBuffer
{
    mat4 pose[];
};

#define NUM_INFLUENCES 4

struct weight{
	uint joint_index;
	float influence;
};

struct VertexJointBinding {
	weight weights[NUM_INFLUENCES];
};

layout(std430, binding = 3) readonly buffer weightsBuffer
{
	VertexJointBinding weights[];				// index, influence
};


layout(push_constant) uniform UniformBufferObject{
	uint numObjects;
	uint numVertices;
	uint numBones;
	uint boneReadOffset;
	uint vertexWriteOffset;	
	uint vertexReadOffset;
} ubo;


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
		
		VertexJointBinding weightsForThisVert = weights[vertID];
		for(uint i = 0; i < NUM_INFLUENCES; i++){
			const weight weightdataBuffer = weightsForThisVert.weights[i];

			uint joint_idx = weightdataBuffer.joint_index;
			float weight = weightdataBuffer.influence;

			//get the pose and bindpose of the target joint
			mat4 posed_mtx = pose[bone_begin + joint_idx];
			totalmtx += weight * posed_mtx;	
		}
		
		//destination to write the matrix
		const uint writeOffset = (vertID + objID * numVerts) + ubo.vertexWriteOffset;	//1x mat4 elements per object
	
		const uint readOffset = (vertID) + ubo.vertexReadOffset;	// there is only one copy of the vertex read data because it comes from the unified mesh data buffer

		// read and apply
		VertexNormalUV readVertex = inputVerts[readOffset];
		vec3 pos = vec3(readVertex.pos_1, readVertex.pos_2, readVertex.pos_3);
		pos = (totalmtx * vec4(pos,1)).xyz;
		vec3 normal = vec3(readVertex.normal_1, readVertex.normal_2, readVertex.normal_3);
		normal = mat3(totalmtx) * normal;
		readVertex.pos_1 = pos.x;
		readVertex.pos_2 = pos.y;
		readVertex.pos_3 = pos.z;
		readVertex.normal_1 = normal.x;
		readVertex.normal_2 = normal.y;
		readVertex.normal_3 = normal.z;

		//write matrix
		vertexOutput[writeOffset] = readVertex;
	}
}
