#extension GL_EXT_samplerless_texture_functions : enable
// adapted from: https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom

layout(push_constant, std430) uniform UniformBufferObject{
    ivec4 targetDim;
} ubo;

layout(binding = 0) uniform texture2D srcTexture;
layout(binding = 1) uniform sampler srcSampler;

layout (location = 0) out vec4 out_downsample;

void main()
{
    vec2 uv = gl_FragCoord.xy / ubo.targetDim.zw;
    
    //rgb + ao
    vec4 p1 = textureOffset(sampler2D(srcTexture, srcSampler), uv, ivec2(0, 0));
	vec4 p2 = textureOffset(sampler2D(srcTexture, srcSampler), uv, ivec2(0, 1));
	vec4 p3 = textureOffset(sampler2D(srcTexture, srcSampler), uv, ivec2(1, 1));
	vec4 p4 = textureOffset(sampler2D(srcTexture, srcSampler), uv, ivec2(1, 0));
 
    // take the max
    float ao = max(max(p1.a, p2.a), max(p3.a, p4.a));

    vec3 col = vec3(
        max(max(p1.r, p2.r), max(p3.r, p4.r)),
        max(max(p1.g, p2.g), max(p3.g, p4.g)),
        max(max(p1.b, p2.b), max(p3.b, p4.b))
    );

	out_downsample = vec4(col,ao);
}
