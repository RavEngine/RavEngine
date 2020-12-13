$input colorintensity, position

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
	float radius = position[3];
	
	vec3 albedo = texture2D(s_albedo, texcoord);
	vec3 normal = texture2D(s_normal, texcoord);
	vec3 pos = texture2D(s_pos, texcoord);
	
	//vec3 lightPosView = mul(u_modelView, vec4(position) );

	vec3 toLight = pos - position;
	
	float dst = distance(pos,position);
	float denom = (dst/radius+1);
	float attenuation = 1.0/(denom * denom);
	
	toLight = normalize(toLight);
	
	float nDotL = max(dot(normal, toLight), 0);
	
	vec3 diffuseLight = albedo * nDotL;
	
	gl_FragData[0] = intensity * attenuation * vec4(diffuseLight, 1.0) * colorintensity;
}
