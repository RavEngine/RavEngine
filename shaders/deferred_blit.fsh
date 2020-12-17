
#include "common.sh"
SAMPLER2D(s_albedo,0);
SAMPLER2D(s_normal,1);
SAMPLER2D(s_light,3);

void main()
{
	vec2 texcoord = vec2(gl_FragCoord.x / u_viewRect[2], gl_FragCoord.y / u_viewRect[3]);
	
	vec3 light = texture2D(s_light, texcoord);
    gl_FragColor = vec4(light,1);
}
