$input a_position

#include <bgfx_shader.sh>

void main()
{
	gl_Position = mul(u_viewProj, vec4(a_position,1));
	gl_Position.z *= -1;
	gl_Position.y *= -1;
}
