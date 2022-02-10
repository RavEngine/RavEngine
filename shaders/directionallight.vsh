$input a_position, i_data0, i_data1
$output lightdir, colorintensity

#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_RW(lightdata,float,11);
uniform vec4 NumObjects;		// x = start index

void main()
{
	colorintensity = i_data0;
	lightdir = i_data1;
	int idx = gl_InstanceID * 3;
	lightdata[NumObjects.x + idx] = lightdir.x;
	lightdata[NumObjects.x + idx + 1] = lightdir.y;
	lightdata[NumObjects.x + idx + 2] = lightdir.z;
	gl_Position = vec4(a_position.xy,1, 1.0);	//depth = 1
}
