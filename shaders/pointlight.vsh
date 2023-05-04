

layout(location = 0) in vec3 a_position;
// per-instance data attributes
layout(location = 1) in vec4 inTransformR1;
layout(location = 2) in vec4 inTransformR2;
layout(location = 3) in vec4 inTransformR3;
layout(location = 4) in vec4 inTransformR4;
layout(location = 5) in vec4 inColorIntensity;


layout(location = 0) out vec4 outColorIntensity;
layout(location = 1) out vec4 outPositionRadius;


layout(push_constant) uniform UniformBufferObject{
	mat4 viewProj;
	mat4 invViewProj;
	ivec4 viewRect;
} ubo;

void main()
{
    
	//get transform data for model matrix
	mat4 model = mat4(
		inTransformR1,
		inTransformR2,
		inTransformR3,
		inTransformR4
	);

	//calculate the radius
	
	float intensity = inColorIntensity[3];
	float radius = intensity * intensity;
    
    model = model * mat4(
                        vec4(radius,0,0,0),
                        vec4(0,radius,0,0),
                        vec4(0,0,radius,0),
                        vec4(0,0,0,1)
                        );
		
	vec4 worldpos = model * vec4(a_position, 1.0);
	
	gl_Position = ubo.viewProj * worldpos;
    
	outColorIntensity = inColorIntensity;
	outPositionRadius = vec4(model[3][0], model[3][1], model[3][2], radius);
}
