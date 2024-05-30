
layout(push_constant) uniform UniformBufferObject{
	mat4 viewProj;
	//mat4 model;
	//float timeSinceStart;
} ubo;

SkyboxVertexOut vert(mat3 rotScaleOnly){
	SkyboxVertexOut vs_out;

   	vs_out.position = vec4(rotScaleOnly * inPosition,1);
	
	// render behind everything
	vs_out.position.z = 0;
	vs_out.position.w = 1;

	return vs_out;
}

