$input a_position, a_normal, a_texcoord0
$output v_normal, v_texcoord0

#include "common.sh"

void main()
{
	v_normal = a_normal;
	v_texcoord0 = a_texcoord0;
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
}
