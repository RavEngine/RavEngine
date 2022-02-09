$input a_position

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>

BUFFER_RO(vertbuffer,float,12);
BUFFER_RO(indbuffer,int,13);

vec3 calcNormal(vec3 u, vec3 v){

	vec3 n = vec3(0,0,0);
	n.x = u.y * v.z - u.z * v.y;
	n.y = u.z * v.x - u.x * v.z;
	n.z = u.x * v.y - u.y * v.x;

	return n;
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
	int index = indbuffer[gl_InstanceID * 3];
	vec3 centerpoint = vec3(vertbuffer[index*3],vertbuffer[index*3+1],vertbuffer[index*3+2]);

	index = indbuffer[gl_InstanceID * 3 + 1];
	vec3 point1 = vec3(vertbuffer[index*3],vertbuffer[index*3+1],vertbuffer[index*3+2]);

	index = indbuffer[gl_InstanceID * 3 + 2];
	vec3 point2 = vec3(vertbuffer[index*3],vertbuffer[index*3+1],vertbuffer[index*3+2]);


	// vectors that compose the base edge, base edge 2, and edge to top base
	mat3 C = mtxFromCols(vec3(1,0,0),vec3(0,0,1),vec3(0,1,0));

	// vectors that compose the deltas from the triangle root, and the normal vector
	mat3 B = mtxFromCols(point1 - centerpoint, point2 - centerpoint, calcNormal(point1 - centerpoint, point2 - centerpoint));

	mat3 A = C * inverse(B);

	mat4 totalMat = mtxFromCols(vec4(A[0],0), vec4(A[1],0), vec4(A[2],0), vec4(centerpoint,1));

	vec4 pos = instMul(totalMat, vec4(a_position + centerpoint,1));

	gl_Position = mul(u_viewProj, pos );
}
