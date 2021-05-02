#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_WR(skinmatrix, vec4, 0);	// 4x4 matrices
BUFFER_RO(bindpose, vec4, 1);	// 4x4 matrices
BUFFER_RO(weights, vec2, 2);	// index, influence
BUFFER_RO(pose, vec4, 3);	 	// 4x4 matrices

uniform vec4 NumObjects;		// x = num objects, y = num vertices

const int NUM_INFLUENCES = 4;

NUM_THREADS(8, 32, 1)
void main()
{
	//prevent out-of-bounds
	if (gl_GlobalInvocationID.y >= NumObjects.y || gl_GlobalInvocationID.x >= NumObjects.x){
		return;
	}
	
	const int weightsid = gl_GlobalInvocationID.y * 4;		//4x vec2 elements elements per vertex
	
	//for reuse
	const mat4 identity = mtxFromRows(vec4(1,0,0,0),vec4(0,1,0,0),vec4(0,0,1,0),vec4(0,0,0,1));
	
	//will become the pose matrix
	mat4 totalmtx = identity;
	
	for(int i = 0; i < NUM_INFLUENCES; i++){
		const vec2 weightdata = weights[weightsid + i];
		const int joint_idx = weightdata.x;
		const float weight = weightdata.y;
		
		//get the pose and bindpose of the target joint
		mat4 posed_mtx;
		mat4 bindpose_mtx;
		for(int j = 0; j < 4; j++){
			posed_mtx[j] = pose[joint_idx * 4 + j];
			bindpose_mtx[j] = bindpose[joint_idx * 4 + j];
		}
		
		//calculate and sum
		totalmtx += (posed_mtx * bindpose_mtx) * weight;
	}
	
	//destination to write the matrix
	const int offset = gl_GlobalInvocationID.x * NumObjects.x * 4 + gl_GlobalInvocationID.y * 4;	//4x vec4s elements per object
	
	//write matrix
	for(int i = 0; i < 4; i++){
		skinmatrix[offset+i] = totalmtx[i];
	}
}
