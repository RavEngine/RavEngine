$input a_position, i_data0, i_data1, i_data2, i_data3
$output colorintensity, positionradius

#include "common.sh"
#include <bgfx_shader.sh>

void main()
{
	//get transform data for model matrix
	mat4 model;
	model[0][0] = i_data0[0];
	model[0][1] = i_data0[1];
	model[0][2] = i_data0[2];
	model[0][3] = 0;
	
	model[1][0] = i_data0[3];
	model[1][1] = i_data1[0];
	model[1][2] = i_data1[1];
	model[1][3] = 0;
	
	model[2][0] = i_data1[2];
	model[2][1] = i_data1[3];
	model[2][2] = i_data2[0];
	model[2][3] = 0;
	
	model[3][0] = i_data2[1];
	model[3][1] = i_data2[2];
	model[3][2] = i_data2[3];
	model[3][3] = 1;
	
	//calculate the radius
	colorintensity = i_data3;
	
	float intensity = colorintensity[3];
	float radius = intensity * intensity;
		
	vec4 worldpos = instMul(model, vec4(a_position, 1.0));
	
	gl_Position = mul(u_viewProj, worldpos);
	
	positionradius = vec4(model[3][0], model[3][1], model[3][2], radius);
}
