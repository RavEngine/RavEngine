$input v_texcoord0

#include "common.sh"
SAMPLER2D(s_albedo,0);
SAMPLER2D(s_normal,1);
SAMPLER2D(s_light,3);

void main()
{
	vec3 light = texture2D(s_light, v_texcoord0);
    gl_FragColor = vec4(light,1);
}
