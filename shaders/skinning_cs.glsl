#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_WR(skinmatrix, vec4, 0);
BUFFER_RO(bindpose, vec4, 1);
BUFFER_RO(weights, vec2, 2);
BUFFER_RO(pose, vec4, 3);

uniform vec4 NumObjects;

const int NUM_INFLUENCES = 4;

NUM_THREADS(16, 16, 1)
void main()
{
	//prevent out-of-bounds
	if (gl_GlobalInvocationID.y > NumObjects.x || gl_GlobalInvocationID.x > NumObjects.y){
		return;
	}
	
	const int weightsid = gl_GlobalInvocationID.y * NumObjects.y * 4 + gl_GlobalInvocationID.x * 4;
	const int vertoffset = gl_GlobalInvocationID.y * NumObjects.y * 2 + gl_GlobalInvocationID.x;
	
	mat4 totalmtx = mtxFromRows(
								vec4(1,0,0,0),
								vec4(0,1,0,0),
								vec4(0,0,1,0),
								vec4(0,0,0,1)
								);
	
	for(int i = 0; i < NUM_INFLUENCES; i++){
		float weight = weights[weightsid + i + 1];
		int joint_idx = weights[weightsid + i * 4] ;
		
		mat4 posed_mtx;
		posed_mtx[0] = pose[joint_idx];
		posed_mtx[1] = pose[joint_idx+1];
		posed_mtx[2] = pose[joint_idx+2];
		posed_mtx[3] = pose[joint_idx+3];
		
		mat4 bindpose_mtx;
		bindpose_mtx[0] = bindpose[vertoffset];
		bindpose_mtx[1] = bindpose[vertoffset+1];
		bindpose_mtx[2] = bindpose[vertoffset+2];
		bindpose_mtx[3] = bindpose[vertoffset+3];
		
		posed_mtx = posed_mtx * bindpose_mtx;
		
		//totalmtx += (posed_mtx * weight);
	}
	//destination to write the matrix
	const int offset = gl_GlobalInvocationID.y * NumObjects.y * 4 + gl_GlobalInvocationID.x * 4;

	skinmatrix[offset] = totalmtx[0];
	skinmatrix[offset+1] = totalmtx[1];
	skinmatrix[offset+2] = totalmtx[2];
	skinmatrix[offset+3] = totalmtx[3];
}
