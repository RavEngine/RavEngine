
#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_WR(skinmatrix, vec4, 0);

NUM_THREADS(512, 1, 1)
void main()
{
	mat4 identity = mtxFromRows(
		vec4(1,0,0,0),
		vec4(0,1,0,0),
		vec4(0,0,1,0),
		vec4(0,0,0,1)
	)
	skinmatrix[gl_GlobalInvocationID.x][gl_GlobalInvocationID.y] = identity;
}