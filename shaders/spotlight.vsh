$input a_position, i_data0, i_data1, i_data2, i_data3, i_data4
$output colorintensity, positionradius, penumbra, lightID, forward

#include "common.sh"
#include <bgfx_shader.sh>
#include <bgfx_compute.sh>

BUFFER_RO(lightdata,float,11);
uniform vec4 NumObjects;		// x = start index

void main()
{
	int idx = NumObjects.x + gl_InstanceID * 16;

	//get transform data for model matrix
	mat4 model;
	
	model[0][0] = lightdata[idx + 0];
	model[0][1] = lightdata[idx + 1];
	model[0][2] = lightdata[idx + 2];
	model[0][3] = 0;

	model[1][0] = lightdata[idx + 3];
	model[1][1] = lightdata[idx + 4];
	model[1][2] = lightdata[idx + 5];
	model[1][3] = 0;

	model[2][0] = lightdata[idx + 6];
	model[2][1] = lightdata[idx + 7];
	model[2][2] = lightdata[idx + 8];
	model[2][3] = 0;

	model[3][0] = lightdata[idx + 9];
	model[3][1] = lightdata[idx + 10];
	model[3][2] = lightdata[idx + 11];
	model[3][3] = 1;
		
	//the intenisty is passed in along with the color
	colorintensity = vec4(lightdata[idx + 12], lightdata[idx + 13], lightdata[idx + 14], lightdata[idx + 16]);
	float coneAngle = lightdata[idx+15];

	// scale the cone by the cone angle, where a radius of 1.0 corresponds to a cone angle of 45 degrees
	float scaleFactor = tan(radians(coneAngle));
	a_position.x *= scaleFactor;
	a_position.z *= scaleFactor;

	// extend the cone by intensity
	// the top of the cone is at (0,0,0) and so does not get scaled
	float len = length(a_position);
	a_position *= (colorintensity.w / (len == 0 ? 1 : len)) * 2;
	
	vec4 worldpos = instMul(model, vec4(a_position, 1.0));
	
	gl_Position = mul(u_viewProj, worldpos);

	mat3 rotScaleOnly = transpose(model);

	forward = normalize(mul(rotScaleOnly, vec4(0, -1, 0, 1)));	// spot lights point down by default
	
	positionradius = vec4(model[3][0], model[3][1], model[3][2], coneAngle);
	float penumbraAngle = radians(lightdata[idx + 17]);
	penumbra = sin(coneAngle - penumbraAngle);	// to avoid calculating in each pixel
	lightID = gl_InstanceID;
}
