layout(location = 0) in vec3 inPosition;
// per-instance data attributes
layout(location = 1) in vec4 inTransformR1;
layout(location = 2) in vec4 inTransformR2;
layout(location = 3) in vec4 inTransformR3;
layout(location = 4) in vec4 inTransformR4;
layout(location = 5) in vec4 inColorIntensity;
layout(location = 6) in vec2 inConeAngleAndPenumbra;	// [0] = cone angle, [1] = penumbra angle

layout(location = 0) out vec4 outColorIntensity;
layout(location = 1) out vec4 outPositionRadius;
layout(location = 2) out float outPenumbra;
layout(location = 3) out vec3 forward;

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

		
	float coneAngle = inConeAngleAndPenumbra.x;

	// scale the cone by the cone angle, where a radius of 1.0 corresponds to a cone angle of 45 degrees
	float scaleFactor = tan(radians(coneAngle));
	vec3 a_position = inPosition;
	a_position.x *= scaleFactor;
	a_position.z *= scaleFactor;

	// extend the cone by intensity
	// the top of the cone is at (0,0,0) and so does not get scaled
	float len = length(a_position);
	a_position *= (inColorIntensity.w / (len == 0 ? 1 : len)) * 2;
	
	vec4 worldpos = model * vec4(a_position, 1.0);
	
	gl_Position = ubo.viewProj * worldpos;

	mat3 rotScaleOnly = transpose(mat3(model));

	forward = normalize(rotScaleOnly * vec3(0, -1, 0));	// spot lights point down by default
	
	outPositionRadius = vec4(worldpos.xyz, cos(radians(coneAngle)));
	float penumbraAngle = radians(inConeAngleAndPenumbra.y);
	outPenumbra = cos(radians(coneAngle) - penumbraAngle);	// to avoid calculating in each pixel
	outColorIntensity = inColorIntensity;
}