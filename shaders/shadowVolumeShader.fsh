$input plane1, plane2, planeCap, planeData

#include "common.sh"

SAMPLER2D(s_depth,1);
SAMPLER2D(s_shadowself,2);

float solvePlane(vec2 pt, vec4 pln){
	return (pln.x*pt.x + pln.y*pt.y + pln.w) / (pln.z);
}

bool numIsBetween(float num, vec2 bounds){
	return (num >= bounds.x && num <= bounds.y) || (num >= bounds.y && num <= bounds.x);
}

void main()
{
	//Convert the pixel into projection coordinates [-1,1]
	vec2 unitizedPixel = vec2(gl_FragCoord.x / u_viewRect.z, gl_FragCoord.y / u_viewRect.w);
	float geodepth = texture2D(s_depth,  unitizedPixel.x);
	float fragDepth = gl_FragCoord.z;
	// don't shadow onto the skybox
	if (geodepth >= 1){
		discard;
	}

	vec2 sampled = texture2D(s_shadowself, unitizedPixel);
	if (gl_FrontFacing){
		// anything in the green channel?
		if (sampled.g == 0){
			// no, so back tri has not rendered yet
			gl_FragData[1] = vec4(fragDepth,0,0,1);
		}
		else{
			// yes, so lets compare it
			if (numIsBetween(geodepth,vec2(fragDepth, sampled.g))){
				gl_FragData[0] = vec4(1,1,1,1);
			}
		}
	}
	else{
		// anything in the red channel?
		if (sampled.r == 0){
			// no, so front tri has not rendered yet
			gl_FragData[1] = vec4(0,fragDepth,0,1);
		}
		else{
			// yes, so lets compare it
			if (numIsBetween(geodepth,vec2(fragDepth, sampled.r))){
				gl_FragData[0] = vec4(1,1,1,1);
			}
		}
	}
}
