#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_WR(skinmatrix, vec4, 0);	// 4x4 matrices
BUFFER_RO(bindpose, vec4, 1);	// 4x4 matrices
BUFFER_RO(weights, vec2, 2);	// index, influence
BUFFER_RO(pose, vec4, 3);	 	// 4x4 matrices

uniform vec4 NumObjects;		// x = num objects, y = num vertices

const int NUM_INFLUENCES = 4;

//adapted from https://d3cw3dd2w32x2b.cloudfront.net/wp-content/uploads/2015/01/matrix-to-quat.pdf
vec4 mat2quat(mat4 m){
	vec4 q;
	float t;
	if (m[2][2] < 0){
		if (m[0][0] > m[1][1]){
			t = 1 + m[0][0] - m[1][1] - m[2][2];
			q = vec4(t, m[0][1]+m[1][0], m[2][0]+m[0][2],m[1][2]-m[2][1]);
		}
		else{
			t = 1 - m[0][0] + m[1][1] - m[2][2];
			q = vec4(m[0][1]+m[1][0], t, m[1][2]+m[2][1], m[2][0]-m[0][2]);
		}
	}
	else{
		if(m[0][0] < -m[1][1]){
			t = 1 - m[0][0] - m[1][1] + m[2][2];
			q = vec4(m[2][0]+m[0][2], m[1][2]+m[2][1], t, m[0][1]-m[1][0]);
		}
		else{
			t = 1 + m[0][0] + m[1][1] + m[2][2];
			q = vec4(m[1][2]-m[2][1], m[2][0]-m[0][2], m[0][1]-m[1][0], t);
		}
	}
	
	q *= 0.5/sqrt(t);
	return q;
}

mat4 quat2mat(vec4 Q){
	float q0 = Q[0];
	float q1 = Q[1];
	float q2 = Q[2];
	float q3 = Q[3];
	
	// First row of the rotation matrix
	float r00 = 2 * (q0 * q0 + q1 * q1) - 1;
	float r01 = 2 * (q1 * q2 - q0 * q3);
	float r02 = 2 * (q1 * q3 + q0 * q2);
	
	// Second row of the rotation matrix
	float r10 = 2 * (q1 * q2 + q0 * q3);
	float r11 = 2 * (q0 * q0 + q2 * q2) - 1;
	float r12 = 2 * (q2 * q3 - q0 * q1);
	
	// Third row of the rotation matrix
	float r20 = 2 * (q1 * q3 - q0 * q2);
	float r21 = 2 * (q2 * q3 + q0 * q1);
	float r22 = 2 * (q0 * q0 + q3 * q3) - 1;
	
	//matrix
	return mtxFromRows(
					   vec4(r00,r01,r02,0),
					   vec4(r10,r11,r12,0),
					   vec4(r20,r21,r22,0),
					   vec4(0,0,0,0)
					   );
	
}

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
	
	mat4 allmats[NUM_INFLUENCES];
	float allweights[NUM_INFLUENCES];
	
	for(int i = 0; i < NUM_INFLUENCES; i++){
		const vec2 weightdata = weights[weightsid + i];
		const int joint_idx = weightdata.x;
		const float weight = weightdata.y;
		allweights[i] = weight;
		
		//get the pose and bindpose of the target joint
		mat4 posed_mtx;
		mat4 bindpose_mtx;
		for(int j = 0; j < 4; j++){
			posed_mtx[j] = pose[joint_idx * 4 + j];
			bindpose_mtx[j] = bindpose[joint_idx * 4 + j];
		}
		
		allmats[i] = posed_mtx * bindpose_mtx;
	}
	
	//calculate weighted transform
	vec4 avg_translate = vec4(0,0,0,0);
	vec3 avg_scale = avg_translate;
	vec4 avg_rot = vec4(0,0,0,0);
	for(int i = 0; i < NUM_INFLUENCES; i++){
		if (allweights[i] == 0){	//prevent NaN
			continue;
		}
		
		avg_translate += (allmats[i] * vec4(0,0,0,1)) * allweights[i];
		
		vec3 scale = vec3(length(allmats[i][0]), length(allmats[i][1]), length(allmats[i][2]));
		avg_scale += scale * allweights[i];

		mat4 rotation = mtxFromCols(vec4(allmats[i][0].xyz/scale.x,0),
									vec4(allmats[i][1].xyz/scale.y,0),
									vec4(allmats[i][2].xyz/scale.z,0),
									vec4(0,0,0,1));
		vec4 quat = mat2quat(rotation);
		avg_rot += quat * allweights[i];
	}
	avg_translate.w = 1;
	
	// create a single combined matrix
	mat4 trmat = mtxFromRows(vec4(1,0,0,0),vec4(0,1,0,0),vec4(0,0,1,0),avg_translate);
	mat4 scmat = mtxFromRows(vec4(avg_scale.x,0,0,0),vec4(0,avg_scale.y,0,0),vec4(0,0,avg_scale.z,0),vec4(0,0,0,1));
	mat4 rmat = quat2mat(avg_rot);
	
	totalmtx = trmat * rmat * scmat;
	
	//destination to write the matrix
	const int offset = gl_GlobalInvocationID.x * NumObjects.x * 4 + gl_GlobalInvocationID.y * 4;	//4x vec4s elements per object
	
	//write matrix
	for(int i = 0; i < 4; i++){
		skinmatrix[offset+i] = totalmtx[i];
	}
}
