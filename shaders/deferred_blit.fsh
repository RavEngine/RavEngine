
#include "common.sh"
SAMPLER2D(s_light,0);
SAMPLER2D(s_depth,1);

void main()
{
	vec2 texcoord = vec2(gl_FragCoord.x / u_viewRect.z, gl_FragCoord.y / u_viewRect.w);
	
	vec3 light = texture2D(s_light, texcoord);
	float depth = texture2D(s_depth, texcoord).x;
	gl_FragDepth = depth;
    gl_FragColor = vec4(light, depth);
}
