$input a_position, i_data0
$output colorintensity

#include "common.sh"

void main()
{
	colorintensity = i_data0;
	gl_Position = vec4(a_position.xy,1,1);
}
