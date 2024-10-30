
layout(push_constant, std430) uniform UniformBufferObject{
	uint targetDim;
} ubo;

layout(binding = 0) uniform texture2D inImage;
layout(binding = 1) uniform sampler g_sampler;

layout(location = 0) out vec4 outcolor;
void main(){

	vec2 texcoord = vec2(gl_FragCoord.x / ubo.targetDim, gl_FragCoord.y / ubo.targetDim);
#if !defined(RGL_SL_MTL) && !defined(RGL_SL_WGSL)
	outcolor = texture(sampler2D(inImage, g_sampler), texcoord).rgba;
#else
    // Metal and WebGPU do not have reduction samplers, so we need to emulate them in software
    vec4 values = textureGather(sampler2D(inImage, g_sampler), texcoord);
    outcolor = min(min(values.x, values.y), min(values.z, values.w)).rrrr;
#endif

}
