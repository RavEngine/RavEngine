$input a_position
$output v_color

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>

BUFFER_RO(vertbuffer,float,12);
BUFFER_RO(indbuffer,int,13);

vec3 calcNormal(vec3 u, vec3 v){

	return normalize(vec3(
		u.y * v.z - u.z * v.y,
		u.z * v.x - u.x * v.z,
		u.x * v.y - u.y * v.x
	));
}

mat3 inverse(mat3 m){
	float OneOverDeterminant = 1 / (
				+ m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2])
				- m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2])
				+ m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2]));

	mat3 Inverse = mtxFromRows(vec3(0,0,0),vec3(0,0,0),vec3(0,0,0));
	Inverse[0][0] = + (m[1][1] * m[2][2] - m[2][1] * m[1][2]) * OneOverDeterminant;
	Inverse[1][0] = - (m[1][0] * m[2][2] - m[2][0] * m[1][2]) * OneOverDeterminant;
	Inverse[2][0] = + (m[1][0] * m[2][1] - m[2][0] * m[1][1]) * OneOverDeterminant;
	Inverse[0][1] = - (m[0][1] * m[2][2] - m[2][1] * m[0][2]) * OneOverDeterminant;
	Inverse[1][1] = + (m[0][0] * m[2][2] - m[2][0] * m[0][2]) * OneOverDeterminant;
	Inverse[2][1] = - (m[0][0] * m[2][1] - m[2][0] * m[0][1]) * OneOverDeterminant;
	Inverse[0][2] = + (m[0][1] * m[1][2] - m[1][1] * m[0][2]) * OneOverDeterminant;
	Inverse[1][2] = - (m[0][0] * m[1][2] - m[1][0] * m[0][2]) * OneOverDeterminant;
	Inverse[2][2] = + (m[0][0] * m[1][1] - m[1][0] * m[0][1]) * OneOverDeterminant;

	return Inverse;
}

void main()
{
	// get the assigned triangle
	int index = indbuffer[gl_InstanceID * 3];
	vec3 centerpoint = vec3(vertbuffer[index*3],vertbuffer[index*3+1],vertbuffer[index*3+2]);
	index = indbuffer[gl_InstanceID * 3 + 1];
	vec3 point1 = vec3(vertbuffer[index*3],vertbuffer[index*3+1],vertbuffer[index*3+2]);
	index = indbuffer[gl_InstanceID * 3 + 2];
	vec3 point2 = vec3(vertbuffer[index*3],vertbuffer[index*3+1],vertbuffer[index*3+2]);

	// vectors that compose the deltas from the triangle root, and the normal vector
	vec3 delta1 = centerpoint - point1;
	vec3 delta2 = centerpoint - point2;
	vec3 normal = calcNormal(delta1, delta2);

	vec3 points[3];
	points[0] = centerpoint;
	points[1] = point1;
	points[2] = point2;

	gl_Position = mul(u_viewProj, vec4(/*a_position +*/ points[gl_VertexID % 3] + normal * (gl_VertexID < 3 ? 0 : 1.2),1));	// debug: move the prism's origin to the center of the assigned triangle

	// debugging color
	v_color = gl_VertexID < 3 ? vec3(1,0,0) : vec3(0,1,1);		// red = base, blue = cap
}
