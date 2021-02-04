$input a_position, a_normal, a_tangent, a_texcoord0, i_data0, i_data1, i_data2, i_data3
$output v_normal, v_texcoord0, v_worldpos

#include "common.sh"
#include <bgfx_shader.sh>

void main()
{
	mat4 worldmat;
	worldmat[0] = i_data0;
	worldmat[1] = i_data1;
	worldmat[2] = i_data2;
	worldmat[3] = i_data3;
	
	mat3 normalmat = worldmat;	//convert 4x4 to 3x3 for rotating normal
	
	//convert normal to world space
	v_normal = mul(normalmat,a_normal);
	v_texcoord0 = a_texcoord0;
	
	vec4 worldpos = instMul(worldmat, vec4(a_position,1));
	
	v_worldpos = worldpos;
	gl_Position = mul(u_viewProj, worldpos);
}
