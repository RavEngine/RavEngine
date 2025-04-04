
struct VertexNormalUV {
	vec3 pos;
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
	vec2 uv;
};

layout(scalar, binding = 0) buffer posOutBuf
{
	vec3 positionOutput[];
};

layout(scalar, binding = 1) buffer normOutBuf
{
	vec3 normalOutput[];
};

layout(scalar, binding = 2) buffer tanOutBuf
{
	vec3 tangentOutput[];
};

layout(scalar, binding = 3) buffer bitOutBuf
{
	vec3 bitangentOutput[];
};

layout(scalar, binding = 4) buffer uvOutBuf
{
	vec2 uvOutput[];
};


layout(scalar, binding = 10) readonly buffer posInBuf
{
	vec3 positionInput[];
};

layout(scalar, binding = 11) readonly buffer normIn
{
	vec3 normalInput[];
};

layout(scalar, binding = 12) readonly buffer tanIn
{
	vec3 tangentInput[];
};

layout(scalar, binding = 13) readonly buffer bitIn
{
	vec3 bitangentInput[];
};

layout(scalar, binding = 14) readonly buffer uvIn
{
	vec2 uvInput[];
};


layout(std430, binding = 20) readonly buffer poseMatrixBuffer
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

layout(std430, binding = 21) readonly buffer weightsBuffer
{
	VertexJointBinding weights[];				// index, influence
};


layout(push_constant, scalar) uniform UniformBufferObject{
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
		uvOutput[writeOffset] = uvInput[readOffset];

		positionOutput[writeOffset] = (totalmtx * vec4(positionInput[readOffset],1)).xyz;
		normalOutput[writeOffset] = mat3(totalmtx) * normalInput[readOffset];
		tangentOutput[writeOffset] = mat3(totalmtx) * tangentInput[readOffset];
		bitangentOutput[writeOffset] = mat3(totalmtx) * bitangentInput[readOffset];
	}
}
