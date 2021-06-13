#include "common.sh"
#include <bgfx_compute.sh>
#include <bgfx_shader.sh>

BUFFER_RO(rvs_pose, vec4, 11);
uniform vec4 NumObjects;			// x = num objects, y = num vertices, z = num bones active, w = offset into transient buffer

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
#define fs_store(mat) gl_FragData[0] = vec4(mat.color,1); gl_FragData[1] = vec4(mat.normal,1); gl_FragData[2] = vec4(mat.position,1); gl_FragData[3] = vec4(1,0.5,0,1);
#define vs_store() gl_Position = mul(u_viewProj, vec4(v_worldpos,1));

/**
 Calculate the posed (or not) matrices and the normal matrix
 @return the following variables are created: mat4 worldmat, mat3 normalmat
 */
#define vs_genmats() mat4 worldmat = mtxFromRows(i_data0,i_data1,i_data2,i_data3);\
{\
	int offset = (NumObjects.z > 0) * (gl_InstanceID * NumObjects.y * 4 + gl_VertexID.x * 4 + NumObjects.w * 4);\
	mat4 blend = mtxFromRows(rvs_pose[offset],rvs_pose[offset+1],rvs_pose[offset+2],rvs_pose[offset+3]);\
	worldmat = mul(blend, worldmat);\
}\
mat3 normalmat = transpose(worldmat); \
v_normal = normalize(mul(normalmat,a_normal));\
v_worldpos = instMul(worldmat,vec4(a_position,1));

