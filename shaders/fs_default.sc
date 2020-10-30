$input v_normal

#include "common.sh"

void main()
{
	vec4 normal = vec4(v_normal,0);

	vec4 lightdir = vec4(u_view[2][0],u_view[2][1],u_view[2][2],0);
	vec4 lightfactor = dot(normal, lightdir);

	gl_FragColor = lightfactor;	
}
