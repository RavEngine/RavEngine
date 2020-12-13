$input a_position, i_data0, i_data1
$output lightdir, colorintensity

#include "common.sh"

void main()
{
	colorintensity = i_data0;
	lightdir = i_data1;
	gl_Position = vec4(a_position, 1.0);
}
