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

#define store(mat) gl_FragData[0] = vec4(mat.color,1); gl_FragData[1] = vec4(mat.normal,1); gl_FragData[2] = vec4(mat.position,1); gl_FragData[3] = vec4(1,0.5,0,1);
