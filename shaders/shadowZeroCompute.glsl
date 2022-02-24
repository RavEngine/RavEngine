#include <bgfx_compute.sh>

BUFFER_WR(output, float, 0);	
uniform vec4 NumObjects;		// x = length of the buffer

NUM_THREADS(64, 1, 1)	// x = index
void main(){
    if (gl_GlobalInvocationID.x <= NumObjects.x){
        output[gl_GlobalInvocationID.x] = 0;
    }
}