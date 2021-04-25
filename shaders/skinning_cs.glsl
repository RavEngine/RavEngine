#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_WR(skinmatrix, vec4, 0);	// 4x4 matrices
BUFFER_RO(bindpose, vec4, 1);	// 4x4 matrices
BUFFER_RO(weights, vec2, 2);	// index, influence
BUFFER_RO(pose, vec4, 3);	 	// 4x4 matrices
BUFFER_RO(vertexbuffer, vec4, 4);		// x,y,z, nx,ny,nz, u,v
BUFFER_RO(boneparents, float, 5);	//single float indices

uniform vec4 NumObjects;		// x = num objects, y = num vertices

const int NUM_INFLUENCES = 4;

// matrix inverse adapted from https://github.com/glslify/glsl-inverse (MIT License)
mat4 inverse(mat4 m) {
	float
	a00 = m[0][0], a01 = m[0][1], a02 = m[0][2], a03 = m[0][3],
	a10 = m[1][0], a11 = m[1][1], a12 = m[1][2], a13 = m[1][3],
	a20 = m[2][0], a21 = m[2][1], a22 = m[2][2], a23 = m[2][3],
	a30 = m[3][0], a31 = m[3][1], a32 = m[3][2], a33 = m[3][3],
	
	b00 = a00 * a11 - a01 * a10,
	b01 = a00 * a12 - a02 * a10,
	b02 = a00 * a13 - a03 * a10,
	b03 = a01 * a12 - a02 * a11,
	b04 = a01 * a13 - a03 * a11,
	b05 = a02 * a13 - a03 * a12,
	b06 = a20 * a31 - a21 * a30,
	b07 = a20 * a32 - a22 * a30,
	b08 = a20 * a33 - a23 * a30,
	b09 = a21 * a32 - a22 * a31,
	b10 = a21 * a33 - a23 * a31,
	b11 = a22 * a33 - a23 * a32,
	
	det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
	
	return mat4(
				a11 * b11 - a12 * b10 + a13 * b09,
				a02 * b10 - a01 * b11 - a03 * b09,
				a31 * b05 - a32 * b04 + a33 * b03,
				a22 * b04 - a21 * b05 - a23 * b03,
				a12 * b08 - a10 * b11 - a13 * b07,
				a00 * b11 - a02 * b08 + a03 * b07,
				a32 * b02 - a30 * b05 - a33 * b01,
				a20 * b05 - a22 * b02 + a23 * b01,
				a10 * b10 - a11 * b08 + a13 * b06,
				a01 * b08 - a00 * b10 - a03 * b06,
				a30 * b04 - a31 * b02 + a33 * b00,
				a21 * b02 - a20 * b04 - a23 * b00,
				a11 * b07 - a10 * b09 - a12 * b06,
				a00 * b09 - a01 * b07 + a02 * b06,
				a31 * b01 - a30 * b03 - a32 * b00,
				a20 * b03 - a21 * b01 + a22 * b00) / det;
}


NUM_THREADS(16, 16, 1)
void main()
{
	//prevent out-of-bounds
	if (gl_GlobalInvocationID.y > NumObjects.x || gl_GlobalInvocationID.x > NumObjects.y){
		return;
	}
	
	const int weightsid = gl_GlobalInvocationID.y * NumObjects.y * 2;
	
	// get the unposed position of the vertex in model space
	const int vertex_id = gl_GlobalInvocationID.y * NumObjects.y * 4 * 2;
	const vec4 vpos = (vertexbuffer[vertex_id].xyz,1);
	const mat4 vpos_m4 = mtxFromRows(vec4(1,0,0,vpos.x),vec4(0,1,0,vpos.y),vec4(0,0,1,vpos.z),vec4(0,0,0,1));
	
	//for reuse
	const mat4 identity = mtxFromRows(vec4(1,0,0,0),vec4(0,1,0,0),vec4(0,0,1,0),vec4(0,0,0,1));
	
	//will become the pose matrix
	mat4 totalmtx = identity;
	
	for(int i = 0; i < NUM_INFLUENCES; i++){
		const vec2 weightdata = weights[weightsid + i];
		const float weight = weightdata.y;
		const int joint_idx = weightdata.x;
		
		//get the pose and bindpose of the parent joint
		mat4 parent_mtx = identity;
		mat4 parent_bindpose = identity;
		const int parent_idx = boneparents[joint_idx];
		if (parent_idx != -1){
			for(int j = 0; j < 4; j++){
				parent_mtx[j] = pose[parent_idx * 4 + j];
				//parent_bindpose[j] = bindpose[parent_idx * 4 + j];
			}
		}
		parent_bindpose = inverse(parent_mtx);
		
		//get the pose and bindpose of the target joint
		mat4 posed_mtx;
		mat4 bindpose_mtx;
		for(int j = 0; j < 4; j++){
			posed_mtx[j] = pose[joint_idx * 4 + j];
			//bindpose_mtx[j] = bindpose[joint_idx * 4 + j];
		}
		bindpose_mtx = inverse(posed_mtx);

		//calculate the matrix
		mat4 localbindpose = bindpose_mtx - parent_bindpose;
		mat4 localvert = bindpose_mtx - vpos_m4;
		mat4 vertInParent = localbindpose + localvert;
		
		mat4 delta = parent_mtx * vertInParent;
		delta += localbindpose;
		
		totalmtx += (weight * delta);
	}
	
	//destination to write the matrix
	const int offset = gl_GlobalInvocationID.y * NumObjects.y * 4 + gl_GlobalInvocationID.x * 4;
	
	for(int i = 0; i < 4; i++){
		skinmatrix[offset+i] = totalmtx[i];
	}
}
