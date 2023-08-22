
layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform UniformBufferObject{
	mat4 viewProj;
	//mat4 model;
	//float timeSinceStart;
} ubo;

void main()
{
	// we are only interested in the non-translation part of the matrix
	mat3 rotScaleOnly = mat3(ubo.viewProj);
	rotScaleOnly *= mat3(
		2,0,0,
		0,2,0,
		0,0,2
	);

	vec4 screenpos = vec4(rotScaleOnly * inPosition,1);
	
	// set both to 1 to make render behind everything
	screenpos.z = 1;
	screenpos.w = 1;
	gl_Position = screenpos;
}
