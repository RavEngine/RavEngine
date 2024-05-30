
layout(location = 0) in vec3 inPosition;

struct SkyboxVertexOut{
    vec4 position;
};

#include "%s"

void main()
{
	// we are only interested in the non-translation part of the matrix
	mat3 rotScaleOnly = mat3(ubo.viewProj);
	rotScaleOnly *= mat3(
		2,0,0,
		0,2,0,
		0,0,2
	);

    SkyboxVertexOut user_out = vert(rotScaleOnly);
	
	gl_Position = user_out.position;
}
