$input a_position
$output v_color

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>

BUFFER_RO(vertbuffer,float,12);
BUFFER_RO(indbuffer,int,13);
BUFFER_RO(light_databuffer,float,14);	
uniform vec4 NumObjects;		// x = start index into light_databuffer, y = number of total instances, z = number of lights shadows are being calculated for

void main()
{
	// get the assigned triangle
	int index = indbuffer[gl_InstanceID * 3];
	vec3 points[3];

	points[0] = vec3(vertbuffer[index*3],vertbuffer[index*3+1],vertbuffer[index*3+2]);
	index = indbuffer[gl_InstanceID * 3 + 1];
	points[1] = vec3(vertbuffer[index*3],vertbuffer[index*3+1],vertbuffer[index*3+2]);
	index = indbuffer[gl_InstanceID * 3 + 2];
	points[2] = vec3(vertbuffer[index*3],vertbuffer[index*3+1],vertbuffer[index*3+2]);

	// TODO: calculate the vector to translate the higher-order vertices along
	// for directional light, this is just the light's forward vector
	// for spot and point lights, this is the vector from the center of the light to the vertex being processed

	//TODO: this is the directional light implementation, add ifdefs for the other light types
	index = (gl_InstanceID / (int)NumObjects.y) * 3;
	vec3 dirvec = vec3(light_databuffer[index],light_databuffer[index+1],light_databuffer[index+2]) * -7;

	gl_Position = mul(u_viewProj, vec4(points[gl_VertexID % 3] + dirvec * (gl_VertexID < 3 ? 0 : 1),1));

	// debugging color
	v_color = gl_VertexID < 3 ? vec3(1,0,0) : vec3(0,1,1);		// red = base, blue = cap
}
