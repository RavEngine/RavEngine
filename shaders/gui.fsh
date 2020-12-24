$input v_color0, v_texcoord0

#include "common.sh"
#include <bgfx_shader.sh>
SAMPLER2D(s_uitex,0);

void main()
{
	vec3 color = v_color0 * texture2D(s_uitex, v_texcoord0);
	gl_FragColor = vec4(color,1);
}
