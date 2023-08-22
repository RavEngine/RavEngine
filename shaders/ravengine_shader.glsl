
layout(std430, binding = 11) readonly buffer rvsPoseBuffer
{
    vec4 rvs_pose[];
};


uniform vec4 NumObjects;			// x = num objects, y = num vertices, z = num bones active, w = offset into transient buffer
uniform vec4 u_time;                // x = time, y = offset into all_geo, z = number of vertices in this primitive, w = unused

struct PBR{
	vec3 color;
	vec3 normal;
	vec3 position;
};

//default-initialize
PBR make_mat(){
	return PBR( vec3(0,0,0), vec3(0,0,0),vec3(0,0,0));

}

mat4 rvs_dxify(mat4 m){
	#if BGFX_SHADER_LANGUAGE_HLSL
	return transpose(m);
	#else
	return m;
	#endif
}

mat4 mtxFromRows(vec4 r1, vec4 r2, vec4 r3, vec4 r4){
	mat4 mat = mat4(
		r1, r2, r3, r4
	);
	return transpose(mat);
}

/**
 Write PBR values to the MRT textures
 @param mat the PBR mat structure to use for writing data
 */
#define fs_store(mat) gl_FragData[0] = vec4(mat.color,1); gl_FragData[1] = vec4(mat.normal,1); gl_FragData[2] = vec4(mat.position,1); gl_FragData[3] = vec4(1,0.5,0,1);
#define vs_store() gl_Position = u_viewProj *  vec4(v_worldpos,1); {\
int idx = (u_time.y+(gl_InstanceID * u_time.z)+gl_VertexID)*3;\
};

/**
 Calculate the posed (or not) matrices and the normal matrix
 @return the following variables are created: mat4 worldmat, mat3 normalmat
 */
#define vs_genmats() mat4 worldmat = mtxFromRows(i_data0,i_data1,i_data2,i_data3);\
{\
	int offset = (NumObjects.z > 0) * (gl_InstanceID * NumObjects.y * 4 + gl_VertexID.x * 4 + NumObjects.w * 4);\
	mat4 blend = mtxFromRows(rvs_pose[offset],rvs_pose[offset+1],rvs_pose[offset+2],rvs_pose[offset+3]);\
	worldmat = blend * worldmat;\
}\
mat3 normalmat = transpose(worldmat); \
v_normal = normalize(normalmat * a_normal);\
v_worldpos = worldmat * vec4(a_position,1);

