$input v_color0, v_texcoord0

#include "common.sh"
#include <bgfx_shader.sh>
SAMPLER2D(s_uitex,0);

void main()
{
	vec4 color = texture2D(s_uitex, v_texcoord0) * v_color0;
	gl_FragColor = color;
}
