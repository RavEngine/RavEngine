#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_WR(output, vec4, 0);	// 4x4 matrices
BUFFER_RO(skinsoa, float, 1);	// vec3, vec3, vec4 (10 total)
BUFFER_RO(weights, vec2, 2);	// index, influence

uniform vec4 NumObjects;		// x = num objects, y = num vertices

const int NUM_INFLUENCES = 4;

mat4 quat2mat(vec4 Q){
	float qw = Q[3];
	float qx = Q[0];
	float qy = Q[1];
	float qz = Q[2];
	
	const float n = 1.0f/sqrt(qx*qx+qy*qy+qz*qz+qw*qw);
	qx *= n;
	qy *= n;
	qz *= n;
	qw *= n;
	
	
	//matrix
	return mtxFromCols(
					   vec4(1.0f - 2.0f*qy*qy - 2.0f*qz*qz, 2.0f*qx*qy - 2.0f*qz*qw, 2.0f*qx*qz + 2.0f*qy*qw, 0.0f),
					   vec4(2.0f*qx*qy + 2.0f*qz*qw, 1.0f - 2.0f*qx*qx - 2.0f*qz*qz, 2.0f*qy*qz - 2.0f*qx*qw, 0.0f),
					   vec4(2.0f*qx*qz - 2.0f*qy*qw, 2.0f*qy*qz + 2.0f*qx*qw, 1.0f - 2.0f*qx*qx - 2.0f*qy*qy, 0.0f),
					   vec4(0.0f, 0.0f, 0.0f, 1.0f)
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
	
	//calculate weighted transform
	vec3 avg_translate = vec3(0,0,0);
	vec3 avg_scale = vec3(1,1,1);
	vec4 avg_rot = vec4(0,0,0,0);
	
	for(int i = 0; i < NUM_INFLUENCES; i++){
		const vec2 weightdata = weights[weightsid + i];
		const int joint_idx = weightdata.x;
		const float weight = weightdata.y;
		
		//get the pose transformation info for the current joint
		//it's arranged as translate[3], scale[3], rotate[4]
		const int beginidx = joint_idx * 10;
		
		const vec3 translate = vec3(skinsoa[beginidx+0],skinsoa[beginidx+1],skinsoa[beginidx+2]) * weight;
		const vec3 scale = vec3(skinsoa[beginidx+3],skinsoa[beginidx+4],skinsoa[beginidx+5]);
		const vec4 rotate = vec4(skinsoa[beginidx+6],skinsoa[beginidx+7],skinsoa[beginidx+8],skinsoa[beginidx+9]) * weight;
		
		avg_translate += translate;
		avg_rot += rotate;
		//avg_scale += scale;
	}

	// create a single combined matrix
	const mat4 trmat = mtxFromRows(vec4(1,0,0,0),vec4(0,1,0,0),vec4(0,0,1,0),vec4(avg_translate,1));
	const mat4 scmat = mtxFromRows(vec4(avg_scale.x,0,0,0),vec4(0,avg_scale.y,0,0),vec4(0,0,avg_scale.z,0),vec4(0,0,0,1));
	const mat4 rmat = quat2mat(avg_rot);

	//create single final matrix
	totalmtx = trmat * rmat * scmat;
	
	//destination to write the matrix
	const int offset = gl_GlobalInvocationID.x * NumObjects.x * 4 + gl_GlobalInvocationID.y * 4;	//4x vec4s elements per object
	
	//write matrix
	for(int i = 0; i < 4; i++){
		output[offset+i] = totalmtx[i];
	}
}
