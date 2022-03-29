$input plane1, plane2, toLight, planeData

#include "common.sh"

SAMPLER2D(s_depth,0);
SAMPLER2D(s_normal,1);

float solvePlane(vec2 pt, vec4 pln){
	return (pln.x*pt.x - pln.y*pt.y + pln.w) / -(pln.z);
}

bool numIsBetween(float num, vec2 bounds){
	return (num >= bounds.x && num <= bounds.y) || (num >= bounds.y && num <= bounds.x);
}

void main()
{
	//Convert the pixel into projection coordinates [-1,1]
	vec2 texcoord = vec2(gl_FragCoord.x / u_viewRect.z, gl_FragCoord.y / u_viewRect.w);
	vec3 Pixel = vec3((gl_FragCoord.x / u_viewRect.z - 0.5) * 2, (gl_FragCoord.y / u_viewRect.w - 0.5) * 2, texture2D(s_depth,  texcoord).x);

	// don't shadow onto the skyboxm or onto backfaces
	vec3 pixelnormal = texture2D(s_normal, texcoord);
	float nDotL = dot(pixelnormal, toLight);
	if (Pixel.z >= 1 || nDotL < -0.01){
		discard;
	}

	float depths[2];
	depths[0] = solvePlane(Pixel.xy,plane1);
	depths[1] = planeData.x > 1 ? solvePlane(Pixel.xy,plane2) : 1;	// if only 1 plane was pointing away, treat the second plane as at the sky

	// are all backplanes behind this pixel?
	bool greater[2];
	greater[0] = depths[0] > Pixel.z;
	greater[1] = depths[1] > Pixel.z;

	// if they are, then this volume contains the pixel, so shadow it
	if (greater[0] && greater[1]){
		gl_FragColor = vec4(0,1,1,0.1);
	}
	else{
		discard;
	}
	
}
