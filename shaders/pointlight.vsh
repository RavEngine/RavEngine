$input a_position, i_data0, i_data1, i_data2, i_data3, i_data4, i_data5
$output colorintensity, position

#include "common.sh"

void main()
{
	mat4 transform;
	transform[0] = i_data0;
	transform[1] = i_data1;
	transform[2] = i_data2;
	transform[3] = i_data3;
	
	colorintensity = i_data4;
	position = i_data5;
	
	gl_Position = mul(transform, vec4(a_position, 1.0) );
}
