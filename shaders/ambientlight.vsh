$input a_position
$output colorintensity

#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_RO(lightdata, float, 11);
uniform vec4 NumObjects;        // x = start index

void main()
{
    int idx = NumObjects.x + gl_InstanceID * 4; // 3 floats per light

    // color
    colorintensity.r = lightdata[idx];
    colorintensity.g = lightdata[idx + 1];
    colorintensity.b = lightdata[idx + 2];

    // intensity
    colorintensity.a = lightdata[idx + 3];
    gl_Position = vec4(a_position.xy, 1, 1);
}