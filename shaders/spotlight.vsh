$input a_position, i_data0, i_data1, i_data2, i_data3, i_data4
$output colorintensity, positionradius, penumbra

#include "common.sh"
#include <bgfx_shader.sh>
#include <bgfx_compute.sh>

BUFFER_RW(lightdata,float,11);
uniform vec4 NumObjects;		// x = start index

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
	
	vec3 color = i_data3.xyz;
	
	//the intenisty is defined as the scale along the Y axis
	float intensity = i_data4[0];
	
	//the radius is defined as the scale along the X or Z axes
	float radius = i_data3.w;
	
	vec4 worldpos = instMul(model, vec4(a_position, 1.0));

	int idx = gl_InstanceID * 3;
	lightdata[NumObjects.x + idx] = worldpos.x;
	lightdata[NumObjects.x + idx + 1] = worldpos.y;
	lightdata[NumObjects.x + idx + 2] = worldpos.z;
	
	gl_Position = mul(u_viewProj, worldpos);
	
	positionradius = vec4(model[3][0], model[3][1], model[3][2], radius);
	colorintensity = vec4(color,intensity);
	penumbra = i_data4[1];
}
