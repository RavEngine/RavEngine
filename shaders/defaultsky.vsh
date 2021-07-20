$input a_position

#include "common.sh"

void main()
{
	vec4 worldpos = instMul(vec4(a_position, 1),u_model[0]);
	vec4 screenpos = mul(u_viewProj, worldpos);
	
	// set both to 1 to make render behind everything
	screenpos.z = 1;
	screenpos.w = 1;
	gl_Position = screenpos;
}
