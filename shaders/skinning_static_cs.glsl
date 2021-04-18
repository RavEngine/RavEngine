
#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_WR(skinmatrix, vec4, 0);

//NUM_THREADS(512, 1, 1)
void main()
{
/*
	mat4 identity = mtxFromRows(
		vec4(1,0,0,0),
		vec4(0,1,0,0),
		vec4(0,0,1,0),
		vec4(0,0,0,1)
	);
	int offset = gl_GlobalInvocationID.x * 4;
	skinmatrix[offset] = identity[0];
	skinmatrix[offset+1] = identity[1];
	skinmatrix[offset+2] = identity[2];
	skinmatrix[offset+3] = identity[3];
	*/
}