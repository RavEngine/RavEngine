$input a_position, a_normal, a_tangent, a_texcoord0, i_data0, i_data1, i_data2, i_data3
$output v_normal, v_texcoord0, v_worldpos

#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_RO(pose, vec4, 11);
uniform vec4 NumObjects;			// x = num objects, y = num vertices, z = num bones

const int NUM_INFLUENCES = 4;

void main()
{
	mat4 worldmat = mtxFromRows(i_data0,i_data1,i_data2,i_data3);
	
	int offset = gl_InstanceID * NumObjects.y * 4 + gl_VertexID.x * 4;
	mat4 blend = mtxFromRows(pose[offset],pose[offset+1],pose[offset+2],pose[offset+3]);

	worldmat = mul(blend, worldmat);
	mat3 normalmat = transpose(worldmat);	//convert 4x4 to 3x3 for rotating normal
	
	//convert normal to world space
	v_normal = normalize(mul(normalmat,a_normal));
	v_texcoord0 = a_texcoord0;
	
	v_worldpos = instMul(worldmat,vec4(a_position,1));
	
	gl_Position = mul(u_viewProj, vec4(v_worldpos,1));
}
