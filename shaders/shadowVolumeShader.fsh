$input plane1, plane2, planeCap, planeData

#include "common.sh"

SAMPLER2D(s_depth,1);

float solvePlane(vec2 pt, vec4 pln){
	return (pln.x*pt.x - pln.y*pt.y + pln.w) / -(pln.z);
}

bool numIsBetween(float num, vec2 bounds){
	return (num >= bounds.x && num <= bounds.y) || (num >= bounds.y && num <= bounds.x);
}

void main()
{
	//Convert the pixel into projection coordinates [-1,1]
	vec3 Pixel = vec3((gl_FragCoord.x / u_viewRect.z - 0.5) * 2, (gl_FragCoord.y / u_viewRect.w - 0.5) * 2, texture2D(s_depth,  vec2(gl_FragCoord.x / u_viewRect.z, gl_FragCoord.y / u_viewRect.w)).x);

	// don't shadow onto the skybox
	if (Pixel.z >= 1){
		//discard;
	}
	gl_FragColor = vec4(0,1,0,0.3);
	//return;

	float depths[3];
	depths[0] = solvePlane(Pixel.xy,planeCap);
	depths[1] = solvePlane(Pixel.xy,plane1);
	depths[2] = planeData.x > 1 ? solvePlane(Pixel.xy,plane2) : 1;	// if only 1 plane was pointing away, treat the second plane as at the sky

	// are all backplanes behind this pixel?
	bool greater[3];
	greater[0] = depths[0] > Pixel.z;
	greater[1] = depths[1] > Pixel.z;
	greater[2] = depths[2] > Pixel.z;
	
	// if they are, then this volume contains the pixel, so shadow it
	if (/*greater[0] &&*/ greater[1] && greater[2]){
		gl_FragColor = vec4(0,1,0,0.3);
	}
	else{
		//discard;
		gl_FragColor = vec4(1,0,0,0.3);
	}
	
}
