$input v_normal, v_tangent, v_texcoord0

#define READ_MATERIAL

#include "common.sh"
#include <bgfx_shader.sh>

void main()
{
//	PBRMaterial mat = pbrMaterial(v_texcoord0);
//	vec3 N = convertTangentNormal(v_normal, v_tangent, mat.normal);
//	mat.a = specularAntiAliasing(N, mat.a);
//	
//	// save normal in camera space
//	// the other renderers render in world space but the
//	// deferred renderer uses camera space to hide artifacts from
//	// normal packing and to make fragment position reconstruction easier
//	// the normal matrix transforms to world coordinates, so undo that
//	N = mul(u_view, vec4(N, 0.0)).xyz;
//	
//	// pack G-Buffer
//	gl_FragData[0] = vec4(mat.diffuseColor, mat.a);
//	gl_FragData[1] = vec4(packNormal(N), 0.0, 0.0);
//	gl_FragData[2] = vec4(mat.F0, mat.metallic);
//	gl_FragData[3] = vec4(mat.emissive, mat.occlusion);
}
