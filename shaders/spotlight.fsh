$input colorintensity, positionradius, penumbra, lightID, forward

#include "common.sh"
#include <bgfx_compute.sh>
SAMPLER2D(s_albedo,0);
SAMPLER2D(s_normal,1);
SAMPLER2D(s_pos,2);
uniform vec4 NumObjects;		// y = shadows enabled

float remap(float value, float min1, float max1, float min2, float max2) {
  return clamp(min2 + (value - min1) * (max2 - min2) / (max1 - min1),min2,max2);
}

EARLY_DEPTH_STENCIL
void main()
{
	//calculate sampling positions using fragment pos and view dimensions
	vec2 texcoord = vec2(gl_FragCoord.x / u_viewRect[2], gl_FragCoord.y / u_viewRect[3]);

	 // is this pixel visible to the light? if not, discard
	bool enabled = 1;
	if (NumObjects.y){
		// shadow tests here
	}
	
	float intensity = colorintensity[3];
	float coneDotFactor = positionradius.w;
	
	vec3 albedo = texture2D(s_albedo, texcoord);
	vec3 normal = texture2D(s_normal, texcoord);
	vec3 pos = texture2D(s_pos, texcoord);
	
	vec3 toLight = normalize(positionradius.xyz - pos);
	
	float dist = distance(pos,positionradius.xyz);
	
	int falloffpower = 2;	//1 for linear, 2 for quadratic, 3 for cubic, ...
	
	float attenuation = 1/pow(dist,falloffpower);
	
	float nDotL = max(dot(normal, toLight), 0);
	
	vec3 diffuseLight = albedo * nDotL;

	// is it inside the light?
	float pixelAngle = dot(-forward,toLight);
	enabled = enabled && (pixelAngle > coneDotFactor);

	// x is inner, y is outer

	float epsilon = penumbra - coneDotFactor;
	float penumbraFactor = clamp((pixelAngle - coneDotFactor) / epsilon, 0, 1);
	
	gl_FragData[0] = vec4(attenuation * colorintensity.xyz * diffuseLight * penumbraFactor * enabled, enabled);
}
