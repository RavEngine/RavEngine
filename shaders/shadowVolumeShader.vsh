$input a_position

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>

BUFFER_RO(vertbuffer,float,12);
BUFFER_RO(indbuffer,int,13);

void main()
{
	int index = indbuffer[gl_InstanceID * 3+3];
	vec3 centerpoint = vec3(vertbuffer[index*3],vertbuffer[index*3+1],vertbuffer[index*3+2]);
	gl_Position = mul(u_modelViewProj, vec4(a_position + centerpoint, 1.0) );
}
