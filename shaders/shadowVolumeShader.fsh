$input plane1, plane2, planeCap, planeData

#include "common.sh"
#include <bgfx_compute.sh>

SAMPLER2D(s_depth,1);
BUFFER_RW(rvs_scratch,float,11);

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

	int index = (gl_FragCoord.y * 2) + gl_FragCoord.x * 2 ;		// 2 floats per pixel
	if (gl_FrontFacing){
		// anything in the green channel?
		if (rvs_scratch[index+1] == 0){
			// no, so back tri has not rendered yet
			rvs_scratch[index] = fragDepth;
			discard;
		}
		else{
			// yes, so lets compare it
			if (numIsBetween(geodepth,vec2(fragDepth, rvs_scratch[index+1]))){
				gl_FragData[0] = vec4(1,1,1,1);
			}
			else{
				discard;
			}
		}
	}
	else{
		// anything in the red channel?
		if (rvs_scratch[index] == 0){
			// no, so front tri has not rendered yet
			rvs_scratch[index] = fragDepth;		
			discard;
		}
		else{
			// yes, so lets compare it
			if (numIsBetween(geodepth,vec2(fragDepth, rvs_scratch[index]))){
				gl_FragData[0] = vec4(1,1,1,1);
			}
			else{
				discard;
			}
		}
	}
}
