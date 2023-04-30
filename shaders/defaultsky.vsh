#version 460

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform UniformBufferObject{
	mat4 viewProj;
	//mat4 model;
	//float timeSinceStart;
} ubo;

void main()
{
	// translate the skybox to the camera 
	vec3 viewTranslate = ubo.viewProj[3].xyz;

	vec4 screenpos = ubo.viewProj * vec4(inPosition + -viewTranslate, 1);
	
	// set both to 1 to make render behind everything
	screenpos.z = 1;
	screenpos.w = 1;
	gl_Position = screenpos;
}
