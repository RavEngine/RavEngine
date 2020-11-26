$input a_position, a_normal, a_tangent, a_texcoord0
$output v_normal, v_tangent, v_texcoord0

#include "common.sh"
#include <bgfx_shader.sh>

uniform mat3 u_normalMatrix;

void main()
{
	v_normal = mul(u_normalMatrix, a_normal);
	v_tangent = mul(u_model[0], vec4(a_tangent, 0.0)).xyz;
	v_texcoord0 = a_texcoord0;
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
}
