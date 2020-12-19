$input colorintensity, positionradius

#include "common.sh"
#include <bgfx_shader.sh>
SAMPLER2D(s_albedo,0);
SAMPLER2D(s_normal,1);
SAMPLER2D(s_pos,2);

void main()
{
	//calculate sampling positions using fragment pos and view dimensions
	vec2 texcoord = vec2(gl_FragCoord.x / u_viewRect[2], gl_FragCoord.y / u_viewRect[3]);
	
	float intensity = colorintensity[3];
	float radius = positionradius[3];
	
	vec3 albedo = texture2D(s_albedo, texcoord);
	vec3 normal = texture2D(s_normal, texcoord);
	vec3 pos = texture2D(s_pos, texcoord);
	
	vec3 toLight = positionradius.xyz - pos;
	
	float dst = distance(pos,positionradius.xyz);
	float attenuation = sqrt(radius/(dst*dst));
	
	toLight = normalize(toLight);
	
	float nDotL = max(dot(normal, toLight), 0);
	
	vec3 diffuseLight = albedo * nDotL;
		
	gl_FragData[0] = vec4(intensity * attenuation * colorintensity.xyz * diffuseLight, 1.0);
}
