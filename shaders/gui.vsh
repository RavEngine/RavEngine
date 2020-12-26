$input a_position, a_color0, a_texcoord0
$output v_color0, v_texcoord0

#include "common.sh"
#include <bgfx_shader.sh>

void main()
{
	v_color0 = a_color0;
	v_texcoord0	= a_texcoord0;
	gl_Position = mul(u_model[0],vec4(a_position, 0, 1));
}
