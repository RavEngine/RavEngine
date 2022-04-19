$input colorintensity

#include "common.sh"
#include <bgfx_shader.sh>

SAMPLER2D(s_albedo,0);

EARLY_DEPTH_STENCIL
void main()
{
	//calculate sampling positions using fragment pos and view dimensions
	vec2 texcoord = vec2(gl_FragCoord.x / u_viewRect[2], gl_FragCoord.y / u_viewRect[3]);
	
	vec4 lightColor = colorintensity;
	
	float intensity = lightColor[3];
	
	vec3 albedo = texture2D(s_albedo, texcoord);
	
	gl_FragData[0] = vec4(albedo * intensity * lightColor.xyz, 1);
}
