$input a_position
$output colorintensity, positionradius, lightID

#include "common.sh"
#include <bgfx_shader.sh>
#include <bgfx_compute.sh>

BUFFER_RO(lightdata, float, 11);
uniform vec4 NumObjects;		// x = start index

void main()
{
    int idx = NumObjects.x + gl_InstanceID * 16;
    
	//get transform data for model matrix
	mat4 model;
	model[0][0] = lightdata[idx+0];
	model[0][1] = lightdata[idx+1];
	model[0][2] = lightdata[idx+2];
	model[0][3] = 0;
	
	model[1][0] = lightdata[idx+3];
	model[1][1] = lightdata[idx+4];
	model[1][2] = lightdata[idx+5];
	model[1][3] = 0;
	
	model[2][0] = lightdata[idx+6];
	model[2][1] = lightdata[idx+7];
	model[2][2] = lightdata[idx+8];
	model[2][3] = 0;
	
	model[3][0] = lightdata[idx+9];
	model[3][1] = lightdata[idx+10];
	model[3][2] = lightdata[idx+11];
	model[3][3] = 1;
	
	//calculate the radius
	colorintensity = vec4(lightdata[idx+12],lightdata[idx+13],lightdata[idx+14],lightdata[idx+15]);
	
	float intensity = colorintensity[3];
	float radius = intensity * intensity;
		
	vec4 worldpos = instMul(model, vec4(a_position, 1.0));
	
	gl_Position = mul(u_viewProj, worldpos);
    
    lightID = gl_InstanceID;
	
	positionradius = vec4(model[3][0], model[3][1], model[3][2], radius);
}
