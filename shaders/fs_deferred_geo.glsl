$input v_normal, v_tangent, v_texcoord0, v_worldpos

#include "common.sh"
#include <bgfx_shader.sh>
#include "pbr_fs.glsl"

void main()
{
	//PBR material = PBR(,v_normal,v_worldpos);
	PBR material = make_mat();
	material.color = vec3(0,0.5,1);
	material.normal = v_normal;
	material.position = v_worldpos;
	
	store(material);
}

