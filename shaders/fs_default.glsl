$input v_normal, v_texcoord0

#include "common.sh"

SAMPLER2D(s_albedoTex,0);

void main()
{
	vec4 normal = vec4(v_normal,0);

	vec4 lightdir = vec4(u_view[2][0],u_view[2][1],u_view[2][2],0);
	vec4 lightfactor = dot(normal, lightdir);
	
	vec4 color = toLinear(texture2D(s_albedoTex, v_texcoord0) );

	gl_FragColor = lightfactor * color;	
}
