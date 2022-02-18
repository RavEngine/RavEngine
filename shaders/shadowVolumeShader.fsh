$input v_color, v_pos1, v_pos2, v_pos3, v_pos4, v_pos5, v_pos6

#include "common.sh"

float sign (vec2 p1, vec2 p2, vec2 p3)
{
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool pointIsInTriangle (vec2 pt, vec2 v1, vec2 v2, vec2 v3)
{
    float d1, d2, d3;
    bool has_neg, has_pos;

    d1 = sign(pt, v1, v2);
    d2 = sign(pt, v2, v3);
    d3 = sign(pt, v3, v1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

float solvePlane(vec2 pt, vec3 a, vec3 b, vec3 c){

	// get the coefficients
	float A = (b.y-a.y)*(c.z-a.z) - (c.y-a.y)*(b.z-a.z);
	float B = (b.z-a.z)*(c.x-a.x) - (c.z-a.z)*(b.x-a.x);
	float C = (b.x-a.x)*(c.x-a.y) - (c.x-a.x)*(b.y-a.y);
	float D = A*a.x + B*a.y + C*a.z;

	// Ax + By + Cz + D = 0, solved for z

	return (A*pt.x + B*pt.y + D) / (-C);
}

bool numIsBetween(float num, vec2 bounds){
	return (num >= bounds.x && num <= bounds.y) || (num >= bounds.y && num <= bounds.x);
}

void main()
{
	//Figure out which 2 faces of the prism the point overlaps with on the XY plane
	vec3 Pixel = vec3((gl_FragCoord.x / u_viewRect.z - 0.5) * 2, (gl_FragCoord.y / u_viewRect.w - 0.5) * 2, gl_FragCoord.z);

	bool IsInside[5];
	float depthIfInside[5];

	// determine if inside
	{
		// end cap tris
		IsInside[0] = pointIsInTriangle(Pixel.xy, v_pos1.xy, v_pos2.xy, v_pos3.xy);
		IsInside[1] = pointIsInTriangle(Pixel.xy, v_pos4.xy, v_pos5.xy, v_pos6.xy);

		// 3 side quads
		IsInside[2] = pointIsInTriangle(Pixel.xy, v_pos1.xy, v_pos4.xy, v_pos5.xy) || pointIsInTriangle(Pixel.xy, v_pos5.xy, v_pos2.xy, v_pos1.xy);
		IsInside[3] = pointIsInTriangle(Pixel.xy, v_pos6.xy, v_pos4.xy, v_pos1.xy) || pointIsInTriangle(Pixel.xy, v_pos3.xy, v_pos6.xy, v_pos1.xy);
		IsInside[4] = pointIsInTriangle(Pixel.xy, v_pos3.xy, v_pos2.xy, v_pos5.xy) || pointIsInTriangle(Pixel.xy, v_pos3.xy, v_pos5.xy, v_pos6.xy);
	}
	// find the depth by solving the planes
	{
		// end caps
		depthIfInside[0] = IsInside[0] ? solvePlane(Pixel.xy, v_pos1, v_pos2, v_pos3) : 0;
		depthIfInside[1] = IsInside[1] ? solvePlane(Pixel.xy, v_pos4, v_pos5, v_pos6) : 0;

		// 3 side quads
		depthIfInside[2] = IsInside[2] ? solvePlane(Pixel.xy, v_pos1, v_pos4, v_pos5) : 0;
		depthIfInside[3] = IsInside[3] ? solvePlane(Pixel.xy, v_pos6, v_pos4, v_pos1) : 0;
		depthIfInside[4] = IsInside[4] ? solvePlane(Pixel.xy, v_pos3, v_pos2, v_pos5) : 0;	
	}

	// is it between?
	vec2 bounds = vec2(0,0);
	for(int i = 0; i < 5; i++){
		if (IsInside[i]){			// way to do this without so much branching?
			if (bounds.x == 0){
				bounds.x = depthIfInside[i];
			}
			else{
				bounds.y = depthIfInside[i];
				break;
			}
		}
	}

	if (bounds.x != 0 && bounds.y != 0 && numIsBetween(Pixel.z, bounds)){
		gl_FragColor = vec4(0,0,0,1);
	}
	else{
		discard;
	}
}
