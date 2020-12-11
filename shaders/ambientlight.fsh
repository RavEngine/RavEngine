#include "common.sh"
#include <bgfx_shader.sh>

SAMPLER2D(s_albedo,0);

uniform vec4 u_lightColor;	//the 4th component stores the intensity

void main()
{
	//calculate sampling positions using fragment pos and view dimensions
	vec2 texcoord = vec2(gl_FragCoord.x / u_viewRect[2], gl_FragCoord.y / u_viewRect[3]);
	
	float intensity = u_lightColor[3];
	
	vec3 albedo = texture2D(s_albedo, texcoord);
	
	gl_FragData[0] = vec4(albedo * intensity * u_lightColor, 1);
}
