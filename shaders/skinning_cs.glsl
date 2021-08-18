#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_WR(output, vec4, 0);	// 4x4 matrices
BUFFER_RO(pose, vec4, 1);	// 4x4 matrices
BUFFER_RO(weights, vec4, 2);	// index, influence, index2, influence2 (this is done becaue DirectX backend can only index on float4s)

uniform vec4 NumObjects;		// x = num objects, y = num vertices, z = num bones, w = offset into transient buffer
uniform vec4 ComputeOffsets;	// x = output offset, y = unused, z = unused, w = unused

#define NUM_INFLUENCES 4

NUM_THREADS(8, 32, 1)	//x = object #, y = vertex #
void main()
{
	//prevent out-of-bounds
	if (gl_GlobalInvocationID.y < NumObjects.y && gl_GlobalInvocationID.x < NumObjects.x){
		// for readability
		const int numObjects = NumObjects.x;
		const int numVerts = NumObjects.y;
		const int numBones = NumObjects.z;
		const int vertID = gl_GlobalInvocationID.y;
		const int objID = gl_GlobalInvocationID.x;
		
		const int weightsid = vertID * 4;		//4x vec2 elements elements per vertex, is always the same per vertex
		
		const int bone_begin = numBones * objID * 4 + NumObjects.w * 4; //offset to the bone for the correct object
				
		//will become the pose matrix
		mat4 totalmtx = mtxFromRows(vec4(0,0,0,0),vec4(0,0,0,0),vec4(0,0,0,0),vec4(0,0,0,0));
		
		for(int i = 0; i < NUM_INFLUENCES / 2; i++){
			const int idx = weightsid + i;
			const vec4 weightdataBuffer = weights[idx];

			vec2 weightdata[2];
			weightdata[0] = weightdataBuffer.xy;
			weightdata[1] = weightdataBuffer.zw;

			for (int x = 0; x < 2; x++) {
				int joint_idx = weightdata[x].x;
				float weight = weightdata[x].y;

				//get the pose and bindpose of the target joint
				mat4 posed_mtx;
				for (int j = 0; j < 4; j++) {
					posed_mtx[j] = pose[bone_begin + joint_idx * 4 + j];
				}
				totalmtx += weight * posed_mtx;
			}	
		}
		
		//destination to write the matrix
		const int offset = (vertID * 4 + objID * numVerts * 4) + ComputeOffsets.x * 4;	//4x vec4s elements per object
	
		// on DirectX, need to convert from column-major to row-major
		#if BGFX_SHADER_LANGUAGE_HLSL
		//totalmtx = transpose(totalmtx);
		#endif

		//write matrix
		for(int i = 0; i < 4; i++){
			output[offset+i] = totalmtx[i];
		}
	}
}
