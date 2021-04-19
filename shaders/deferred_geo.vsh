$input a_position, a_normal, a_tangent, a_texcoord0, i_data0, i_data1, i_data2, i_data3
$output v_normal, v_texcoord0, v_worldpos

#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_RO(skinmatrix, vec4, 0);

void main()
{
	mat4 worldmat;
	worldmat[0] = i_data0;
	worldmat[1] = i_data1;
	worldmat[2] = i_data2;
	worldmat[3] = i_data3;
	
	mat3 normalmat = transpose(worldmat);	//convert 4x4 to 3x3 for rotating normal
	
	//convert normal to world space
	v_normal = mul(normalmat,a_normal);
	v_texcoord0 = a_texcoord0;
	
	int offset = gl_VertexID.x * 4;
	mat4 posemtx;
	posemtx[0] = skinmatrix[offset];
	posemtx[1] = skinmatrix[offset+1];
	posemtx[2] = skinmatrix[offset+2];
	posemtx[3] = skinmatrix[offset+3];
	
	worldmat = mul(worldmat,posemtx);
	
	vec4 worldpos = instMul(worldmat, vec4(a_position,1));
	
	v_worldpos = worldpos;
	gl_Position = mul(u_viewProj, worldpos);
}
