$input a_position
$output colorintensity

#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_RO(lightdata,vec4,11);
uniform vec4 NumObjects;        // x = start index

void main()
{
    int idx = NumObjects.x + gl_InstanceID * 4; // 4 floats per light, which is exactly one vec4
    colorintensity = lightdata[idx];    // grabs the color and intensity in one go
	gl_Position = vec4(a_position.xy,1,1);
}
