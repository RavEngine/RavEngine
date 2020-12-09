#include "common.sh"
#include <bgfx_shader.sh>
SAMPLER2D(s_albedo,0);
SAMPLER2D(s_normal,1);
SAMPLER2D(s_pos,2);

uniform vec4 u_lightPos;
uniform vec4 u_lightColor;

void main()
{
	//calculate sampling positions using fragment pos and view dimensions
	vec2 texcoord = vec2(gl_FragCoord.x / u_viewRect[2], gl_FragCoord.y / u_viewRect[3]);
	
	vec3 albedo = toLinear(texture2D(s_albedo, texcoord));
	vec3 normal = toLinear(texture2D(s_normal, texcoord));
	vec3 pos = toLinear(texture2D(s_pos, texcoord));
	
	vec3 viewPos = vec3(u_view[2][0], u_view[2][1], u_view[2][2]);
	vec3 viewDir = normalize(viewPos - pos);
	
	vec3 lightDir = normalize(u_lightPos - pos);
	vec3 diffuse = max(dot(normal, lightDir), 0) * albedo * u_lightColor;
	
	gl_FragData[0] = vec4(diffuse,1);
}
