#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_WR(output, vec4, 0);	// 4x4 matrices
BUFFER_RO(pose, vec4, 1);	// 4x4 matrices
BUFFER_RO(weights, vec2, 2);	// index, influence

uniform vec4 NumObjects;		// x = num objects, y = num vertices, z = num bones

const int NUM_INFLUENCES = 4;

NUM_THREADS(8, 32, 1)	//x = object #, y = vertex #
void main()
{
	//prevent out-of-bounds
	if (gl_GlobalInvocationID.y >= NumObjects.y || gl_GlobalInvocationID.x >= NumObjects.x){
		return;
	}
	
	// for readability
	const int numObjects = NumObjects.x;
	const int numVerts = NumObjects.y;
	const int numBones = NumObjects.z;
	const int vertID = gl_GlobalInvocationID.y;
	const int objID = gl_GlobalInvocationID.x;
	
	const int weightsid = vertID * 4;		//4x vec2 elements elements per vertex, is always the same per vertex
	
	const int bone_begin = numBones * objID * 4; //offset to the bone for the correct object
			
	//will become the pose matrix
	mat4 totalmtx = mtxFromRows(vec4(0,0,0,0),vec4(0,0,0,0),vec4(0,0,0,0),vec4(0,0,0,0));
	
	for(int i = 0; i < NUM_INFLUENCES; i++){
		const vec2 weightdata = weights[weightsid + i];
		const int joint_idx = weightdata.x;
		const float weight = weightdata.y;
		
		//get the pose and bindpose of the target joint
		mat4 posed_mtx;
		for(int j = 0; j < 4; j++){
			posed_mtx[j] = pose[bone_begin + joint_idx * 4 + j];
		}
		posed_mtx = mtxFromRows(posed_mtx[0],posed_mtx[1],posed_mtx[2],posed_mtx[3]);
		totalmtx += posed_mtx * weight;
	}
	
	//destination to write the matrix
	const int offset = numVerts * objID * 4 + vertID * 4;	//4x vec4s elements per object
	
	//write matrix
	for(int i = 0; i < 4; i++){
		output[offset+i] = totalmtx[i];
	}
}
