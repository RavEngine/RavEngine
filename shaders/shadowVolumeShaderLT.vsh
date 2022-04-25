$input a_position

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>

BUFFER_RO(vertbuffer,float,12);
BUFFER_RO(indbuffer,int,13);
BUFFER_RO(light_databuffer,float,14);	
uniform vec4 NumObjects;		// x = start index into light_databuffer, y = number of total instances, z = light data stride, w = type of light (0 = dir, 1 = point, 2 = spot)

#define INSET -0.02
#define EXPAND 0.002
#define MINDOTL 0.1

vec3 calcNormal(vec3 a, vec3 b, vec3 c){
	return normalize(cross(b-a,c-a));
}

vec3 calcNormalV4(vec4 a, vec4 b, vec4 c) {
	return calcNormal(a.xyz, b.xyz, c.xyz);
}

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

	vec3 normal = calcNormal(points[0], points[1], points[2]);
    
    vec3 toLight = vec3(0,0,0);

	index = NumObjects.x + (gl_InstanceID / (int)NumObjects.y) * NumObjects.z;
	if (NumObjects.w == 0) {
		// for directional light, this is just the light's forward vector
		vec3 lightDirData = vec3(light_databuffer[index + 4], light_databuffer[index + 1 + 4], light_databuffer[index + 2 + 4]);
		toLight = normalize(lightDirData);
	}
	else {
		// for spot and point lights, this is the vector to the center of the light from the vertex being processed
		vec3 lightpos = vec3(light_databuffer[index+9], light_databuffer[index + 10], light_databuffer[index + 11]);
		toLight = normalize(lightpos - points[gl_VertexID % 3]);
	}

	// if the triangle is facing the wrong way, we don't want to have it cast shadows (reduce the number of volumes generated)
	float nDotL = max(dot(normal, toLight), 0);
	if (nDotL < MINDOTL) {
		gl_Position = vec4(0,0,0,1);	// don't shade this by placing it at the origin
	}
	else
    {
        vec3 center = (points[0] + points[1] + points[2])/3.0;
        vec3 dirvec = (toLight + (normalize(center-points[gl_VertexID % 3])* EXPAND)) * -1000;
        
		// inset the triangle by the normal a small amount, then extend it along the light ray
		gl_Position = mul(u_viewProj, vec4((points[gl_VertexID % 3] + normal * INSET) + dirvec * (gl_VertexID < 3 ? 0 : 1),1));
	}
}
