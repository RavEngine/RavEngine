$input a_position, i_data0, i_data1, i_data2, i_data3, i_data4
$output colorintensity, positionradius, penumbra

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
	
	vec3 color = i_data4.xyz;
	
	//the intenisty is defined as the scale along the Y axis
	float intensity = sqrt(length(vec3(i_data0.y,i_data1.y,i_data2.y)));
	
	//the radius is defined as the scale along the X or Z axes
	float radius = length(vec3(i_data0.x,i_data1.x,i_data2.x));
	
	vec4 worldpos = instMul(model, vec4(a_position, 1.0));
	
	gl_Position = mul(u_viewProj, worldpos);
	
	positionradius = vec4(i_data3.xyz,radius);
	colorintensity = vec4(color,intensity);
	penumbra = vec4(i_data4.w,0,0,0);
}
