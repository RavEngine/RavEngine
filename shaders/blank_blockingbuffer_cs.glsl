#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_RW(lightdata, uint, 1);		// the amalgamated index buffer
uniform vec4 NumObjects;            // x = width, y = height

NUM_THREADS(32, 32, 1)	// x = per index, y = per instance
void main(){
    if (gl_GlobalInvocationID.x < NumObjects.x && gl_GlobalInvocationID.y < NumObjects.y){
        int index = gl_GlobalInvocationID.y * NumObjects.x + gl_GlobalInvocationID.x;      // where in the buffer
        lightdata[index] = 0;   // reset all
    }
}
