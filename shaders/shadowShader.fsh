
#include "common.sh"

SAMPLER2D(s_shadowself,1);

void main()
{
	vec2 unitizedPixel = vec2(gl_FragCoord.x / u_viewRect.z, gl_FragCoord.y / u_viewRect.w);
	vec3 shadowData = texture2D(s_shadowself,  unitizedPixel);
	gl_FragColor = vec4(1-shadowData,1);
}
