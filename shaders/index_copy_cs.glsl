#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_RO(input_indices, int, 0);	// the separated index buffer
BUFFER_RW(all_indices, int, 1);		// the amalgamated index buffer
uniform vec4 NumObjects;			// x = current offset, y = total count for this invocation, z = counting begin,

NUM_THREADS(64, 1, 1)	// x = per index, y = per instance
void main(){
    uint indexID = gl_GlobalInvocationID.x;      // which index are we copying
    uint numIndicesToWrite = NumObjects.y;        // how many indices we are writing

    if (indexID < numIndicesToWrite){    // out of range check

        uint instanceID = gl_GlobalInvocationID.y;   // which instance is this for
        uint beginIndex = NumObjects.x;              // the begin index across all instances for this draw
        uint firstIndexOffset = NumObjects.z;        // the "zero" index
        uint numVertInvocations = NumObjects.w;      // the number of vertices (not indices!) in this draw

		uint input_index = 0;
		if (numIndicesToWrite > 65535 || BGFX_SHADER_LANGUAGE_HLSL){
			input_index = input_indices[indexID];
		}
		else{
			uint newIndex = indexID / 2;
			input_index = input_indices[newIndex];
			
			// shift
			if (indexID % 2 != 0){	// if odd, the data is in the upper bits (endian little hate i)
				input_index = input_index >> 16;
			}
			else{					// if even, the data is in the lower bits
				input_index = input_index & 0x0000FFFF;
			}
		}
		// calculate new index
		uint all_indices_index = instanceID * numIndicesToWrite + indexID + beginIndex;
		uint computedIndex = input_index + firstIndexOffset + instanceID * numVertInvocations;
		all_indices[all_indices_index] = computedIndex;
    }
}
