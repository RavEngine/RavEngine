#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_WR(skinmatrix, vec4, 0);
BUFFER_RO(bindpose, vec4, 1);
BUFFER_RO(weights, vec4, 2);
BUFFER_RO(verts, vec4, 3);

NUM_THREADS(32, 32, 1)
void main()
{
}
