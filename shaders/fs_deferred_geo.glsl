$input v_normal, v_tangent, v_texcoord0

#include "common.sh"
#include <bgfx_shader.sh>

void main()
{
//	// pack G-Buffer
	gl_FragData[0] = vec4(v_normal.xyz,1);
	gl_FragData[1] = vec4(1,0.5,0,1);
	gl_FragData[2] = vec4(1,0.5,0,1);
	gl_FragData[3] = vec4(1,0.5,0,1);

}
