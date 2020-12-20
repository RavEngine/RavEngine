$input lightdir, colorintensity

#include "common.sh"
#include <bgfx_shader.sh>
SAMPLER2D(s_albedo,0);
SAMPLER2D(s_normal,1);

void main()
{
	//calculate sampling positions using fragment pos and view dimensions
	vec2 texcoord = vec2(gl_FragCoord.x / u_viewRect[2], gl_FragCoord.y / u_viewRect[3]);
	
	float intensity = colorintensity[3];
	
	vec3 albedo = texture2D(s_albedo, texcoord);
	vec3 normal = texture2D(s_normal, texcoord);
	
	vec3 toLight = normalize(lightdir.xyz);
	
	float nDotL = max(dot(normal, toLight), 0);
	
	vec3 diffuseLight = albedo * nDotL;
	
	gl_FragData[0] = vec4(intensity * colorintensity.xyz * diffuseLight, 1.0);
}
