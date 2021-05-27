#include "common.sh"
#include <bgfx_shader.sh>

struct PBR{
	vec3 color;
	vec3 normal;
	vec3 position;
};

//default-initialize
PBR make_mat(){
#if BGFX_SHADER_LANGUAGE_HLSL 
	PBR mat;
	return mat;
#else
	return PBR( vec3(0,0,0), vec3(0,0,0),vec3(0,0,0));
#endif
}

/**
 Write PBR values to the MRT textures
 @param mat the PBR mat structure to use for writing data
 */
#define store(mat) gl_FragData[0] = vec4(mat.color,1); gl_FragData[1] = vec4(mat.normal,1); gl_FragData[2] = vec4(mat.position,1); gl_FragData[3] = vec4(1,0.5,0,1);

/**
 Calculate the posed (or not) matrices and the normal matrix
 @param pose the buffer containing the pose matrix. This matrix is calculated internally in a compute shader.
 @param NumObjects the uniform containing NumObjects. This uniform is set internally and used to determine the pose buffer offset to retrieve the skinning matrix.
 @return the following variables are created: mat4 worldmat, mat3 normalmat
 */
#define vs_genmats(pose,NumObjects) mat4 worldmat = mtxFromRows(i_data0,i_data1,i_data2,i_data3);\
{\
	int offset = gl_InstanceID * NumObjects.y * 4 + gl_VertexID.x * 4;\
	mat4 blend = mtxFromRows(pose[offset],pose[offset+1],pose[offset+2],pose[offset+3]);\
	worldmat = mul(blend, worldmat);\
}\
mat3 normalmat = transpose(worldmat); \
v_normal = normalize(mul(normalmat,a_normal));

