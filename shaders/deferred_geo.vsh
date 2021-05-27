$input a_position, a_normal, a_tangent, a_texcoord0, i_data0, i_data1, i_data2, i_data3
$output v_normal, v_texcoord0, v_worldpos

#include "common.sh"
#include <bgfx_compute.sh>
#include "ravengine_shader.glsl"

BUFFER_RO(pose, vec4, 11);
uniform vec4 NumObjects;			// x = num objects, y = num vertices, z = num bones

void main()
{
	// This macro creates the following variables: mat4 worldmat, mat3 normalmat
	// it also applies skinning from the pose matrix
	// It also sets the value of v_normal. You can update it afterward if needed.
	vs_genmats(pose,NumObjects);
	
	// custom texcoord if applicable
	v_texcoord0 = a_texcoord0;
	
	// set vertex pos in world space, this is important for the fragment shader
	v_worldpos = instMul(worldmat,vec4(a_position,1));
	
	//project world vertex to screen space
	gl_Position = mul(u_viewProj, vec4(v_worldpos,1));
}
