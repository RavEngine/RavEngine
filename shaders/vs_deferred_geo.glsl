$input a_position, a_normal, a_tangent, a_texcoord0
$output v_normal, v_texcoord0, v_worldpos

#include "common.sh"
#include <bgfx_shader.sh>

void main()
{
	v_normal = a_normal;
	v_texcoord0 = a_texcoord0;
    v_worldpos = a_position;
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
}
