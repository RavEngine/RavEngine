$input v_texcoord0

#include "common.sh"
SAMPLER2D(s_albedo,0);
//SAMPLER2D(s_normal,1);
//SAMPLER2D(s_pos,2);
SAMPLER2D(s_light,3);

void main()
{
	vec3 albedo = toLinear(texture2D(s_albedo, v_texcoord0));
//	vec3 normal = toLinear(texture2D(s_normal, v_texcoord0));
//	vec3 pos = toLinear(texture2D(s_pos, v_texcoord0));
	vec3 light = toLinear(texture2D(s_light, v_texcoord0));
    gl_FragColor = vec4(light,1);
}
