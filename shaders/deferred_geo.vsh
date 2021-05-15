$input a_position, a_normal, a_tangent, a_texcoord0, i_data0, i_data1, i_data2, i_data3
$output v_normal, v_texcoord0, v_worldpos

#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_RO(pose, vec4, 11);
BUFFER_RO(weights, vec2, 12);		// index, influence
uniform vec4 NumObjects;			// x = num objects, y = num vertices, z = number of bones

const int NUM_INFLUENCES = 4;

void main()
{
	const int weightsid = gl_VertexID.x * 4;		//4x vec2 elements elements per vertex
	mat4 blend = mtxFromRows(vec4(0,0,0,0),vec4(0,0,0,0),vec4(0,0,0,0),vec4(0,0,0,0));
	
	for(int i = 0; i < NUM_INFLUENCES; i++){
		const vec2 weightdata = weights[weightsid + i];
		const int joint_idx = weightdata.x;
		const float weight = weightdata.y;
		
		//get the pose and bindpose of the target joint
		mat4 posed_mtx;
		for(int j = 0; j < 4; j++){
			posed_mtx[j] = pose[joint_idx * 4 + j];
		}
		posed_mtx = mtxFromRows(posed_mtx[0],posed_mtx[1],posed_mtx[2],posed_mtx[3]);
		blend += posed_mtx * weight;
	}
	mat4 worldmat = mtxFromRows(i_data0,i_data1,i_data2,i_data3);
	worldmat = mul(blend, worldmat);
	mat3 normalmat = transpose(worldmat);	//convert 4x4 to 3x3 for rotating normal
	
	//convert normal to world space
	v_normal = normalize(mul(normalmat,a_normal));
	v_texcoord0 = a_texcoord0;
	
	v_worldpos = instMul(worldmat,vec4(a_position,1));
	
	gl_Position = mul(u_viewProj, vec4(v_worldpos,1));
}
