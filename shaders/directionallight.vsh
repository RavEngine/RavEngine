$input a_position
$output lightdir, colorintensity, lightID

#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_RO(lightdata,vec4,11);
uniform vec4 NumObjects;		// x = start index

void main()
{
    int idx = (NumObjects.x + gl_InstanceID * 7)/4;    // 4 floats per light

    // color & intensity
    colorintensity = lightdata[idx];

    // direction
    lightdir = lightdata[idx + 1].xyz;
    
    // lightID
    lightID = gl_InstanceID;

	gl_Position = vec4(a_position.xy,1, 1.0);	//depth = 1
}
