$input a_position, i_data0, i_data1, i_data2, i_data3, i_data4
$output colorintensity, positionradius

#include "common.sh"
#include <bgfx_shader.sh>

void main()
{
	//get transform data for model matrix
	mat4 model;
	model[0] = i_data0;
	model[1] = i_data1;
	model[2] = i_data2;
	model[3] = i_data3;
	
	//calculate the radius
	colorintensity = i_data4;
	
	float intensity = colorintensity[3];
	float radius = intensity * intensity;
		
	vec4 worldpos = instMul(model, vec4(a_position, 1.0));
	
	gl_Position = mul(u_viewProj, worldpos);
	
	positionradius = vec4(i_data3.xyz,radius);
}
