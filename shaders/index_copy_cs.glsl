#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_RO(input_indices, int, 0);	// the separated index buffer
BUFFER_RW(all_indices, int, 1);		// the amalgamated index buffer
uniform vec4 NumObjects;			// x = current offset, y = total count for this invocation, z = counting begin,

NUM_THREADS(64, 1, 1)	// x = per index, y = per instance
void main(){
    int indexID = gl_GlobalInvocationID.x;      // which index are we copying
    int numIndicesToWrite = NumObjects.y;        // how many indices we are writing

    if (indexID < numIndicesToWrite){    // out of range check

        int instanceID = gl_GlobalInvocationID.y;   // which instance is this for
        int beginIndex = NumObjects.x;              // the begin index across all instances for this draw
        int firstIndexOffset = NumObjects.z;        // the "zero" index
        int numVertInvocations = NumObjects.w;      // the number of vertices (not indices!) in this draw

        // begin index + 
	    all_indices[instanceID * numIndicesToWrite + indexID + beginIndex] = input_indices[indexID] + firstIndexOffset + instanceID * numVertInvocations;
    }
}