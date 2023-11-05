
layout(push_constant, std430) uniform UniformBufferObject{
	uint targetDim;
} ubo;

layout(binding = 0) uniform texture2D inImage;
layout(binding = 1) uniform sampler g_sampler;

layout(location = 0) out vec4 outcolor;
void main(){

	vec2 texcoord = vec2(gl_FragCoord.x / ubo.targetDim, gl_FragCoord.y / ubo.targetDim);

	outcolor = texture(sampler2D(inImage, g_sampler), texcoord).rgba;

}