$input plane1, plane2, planeCap, planeData

#include "common.sh"

SAMPLER2D(s_depth,1);

float solvePlane(vec2 pt, vec4 pln){
	return (pln.x*pt.x + pln.y*pt.y + pln.w) / pln.z;
}

bool numIsBetween(float num, vec2 bounds){
	return (num >= bounds.x && num <= bounds.y) || (num >= bounds.y && num <= bounds.x);
}

void main()
{
	//Figure out which 2 faces of the prism the point overlaps with on the XY plane
	vec3 Pixel = vec3((gl_FragCoord.x / u_viewRect.z - 0.5) * 2, (gl_FragCoord.y / u_viewRect.w - 0.5) * 2, texture2D(s_depth,  vec2(gl_FragCoord.x / u_viewRect.z, gl_FragCoord.y / u_viewRect.w)).x);

	float depths[3];
	depths[0] = solvePlane(Pixel.xy,planeCap);
	depths[1] = solvePlane(Pixel.xy,plane1);
	depths[2] = solvePlane(Pixel.xy,plane2);

	bool greater[3];
	greater[0] = depths[0] > Pixel.z;
	greater[1] = depths[1] > Pixel.z;
	greater[2] = planeData.x <= 1 || depths[2] > Pixel.z;

	if (greater[0] && greater[1] && greater[2]){
		gl_FragColor = vec4(0,0,0,1);
	}
	else{
		discard;
	}

}
