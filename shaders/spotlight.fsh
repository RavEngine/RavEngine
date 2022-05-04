$input colorintensity, positionradius, penumbra, lightID

#include "common.sh"
#include <bgfx_compute.sh>
SAMPLER2D(s_albedo,0);
SAMPLER2D(s_normal,1);
SAMPLER2D(s_pos,2);
BUFFER_RO(blockingDataBuf, uint, 10);
uniform vec4 NumObjects;		// y = shadows enabled

EARLY_DEPTH_STENCIL
void main()
{
	//calculate sampling positions using fragment pos and view dimensions
	vec2 texcoord = vec2(gl_FragCoord.x / u_viewRect[2], gl_FragCoord.y / u_viewRect[3]);

	 // is this pixel visible to the light? if not, discard
	bool enabled = 1;
	if (NumObjects.y){
		uint visibilityMask = blockingDataBuf[gl_FragCoord.y * u_viewRect.z + gl_FragCoord.x];
		uint thisLight = 1 << lightID;
		enabled = !(visibilityMask & thisLight);   // if the light is blocked, do not light here
	}
	
	float intensity = colorintensity[3];
	float radius = positionradius[3];
	
	vec3 albedo = texture2D(s_albedo, texcoord);
	vec3 normal = texture2D(s_normal, texcoord);
	vec3 pos = texture2D(s_pos, texcoord);
	
	vec3 toLight = normalize(positionradius.xyz - pos);
	
	float dst = distance(pos,positionradius.xyz);
	
	int falloffpower = 1;	//1 for linear, 2 for quadratic, 3 for cubic, ...
	
	float attenuation = pow(max(radius-dst,0),falloffpower) * (1.0/pow(radius,falloffpower));
	
	float nDotL = max(dot(normal, toLight), 0);
	
	vec3 diffuseLight = albedo * nDotL;
	
	gl_FragData[0] = vec4(intensity * attenuation * colorintensity.xyz * diffuseLight * enabled, enabled);
}
