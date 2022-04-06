$input a_position
$output lightdir, colorintensity, lightID

#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_RO(lightdata, float, 11);
uniform vec4 NumObjects;		// x = start index

void main()
{
    int idx = NumObjects.x + gl_InstanceID * 7;    // 4 floats per light

    // color
    colorintensity.r = lightdata[idx];
    colorintensity.g = lightdata[idx + 1];
    colorintensity.b = lightdata[idx + 2];

    // intensity
    colorintensity.a = lightdata[idx + 3];

    // direction
    lightdir.x = lightdata[idx + 4];
    lightdir.y = lightdata[idx + 5];
    lightdir.z = lightdata[idx + 6];

    // lightID
    lightID = gl_InstanceID;

    gl_Position = vec4(a_position.xy, 1, 1.0);	//depth = 1
}