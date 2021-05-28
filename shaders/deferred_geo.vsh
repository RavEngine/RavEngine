$input a_position, a_normal, a_tangent, a_texcoord0, i_data0, i_data1, i_data2, i_data3
$output v_normal, v_texcoord0, v_worldpos

#include "ravengine_shader.glsl"

void main()
{
	// This macro creates the following variables: mat4 worldmat, mat3 normalmat
	// It applies skinning from the pose matrix to worldmat.
	// It also sets the value of v_normal and v_worldpos by multiplying the matrices. You can update it afterward if needed.
	vs_genmats();
	
	// custom texcoord if applicable
	v_texcoord0 = a_texcoord0;
	
	//project world vertex to screen space, uses the value of v_worldpos
	vs_store();
}
