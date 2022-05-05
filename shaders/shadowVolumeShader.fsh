#include <bgfx_shader.sh>

EARLY_DEPTH_STENCIL 
void main()
{
	gl_FragColor = vec4(gl_PrimitiveID,0,0,0);
}
