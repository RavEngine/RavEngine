#extension GL_EXT_samplerless_texture_functions : enable
// adapted from: https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom

layout(push_constant, std430) uniform UniformBufferObject{
    ivec2 targetDim;
} ubo;

layout(binding = 0) uniform texture2D srcTexture;
layout(binding = 1) uniform sampler srcSampler;

layout (location = 0) out vec4 out_downsample;

void main()
{
    vec3 downsample = vec3(0,0,0);
    vec2 srcResolution = textureSize(srcTexture,0).xy;
    vec2 texCoord = gl_FragCoord.xy / ubo.targetDim;
    vec2 srcTexelSize = 1.0 / srcResolution;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(sampler2D(srcTexture, srcSampler), vec2(texCoord.x - 2*x, texCoord.y + 2*y)).rgb;
    vec3 b = texture(sampler2D(srcTexture, srcSampler), vec2(texCoord.x,       texCoord.y + 2*y)).rgb;
    vec3 c = texture(sampler2D(srcTexture, srcSampler), vec2(texCoord.x + 2*x, texCoord.y + 2*y)).rgb;

    vec3 d = texture(sampler2D(srcTexture, srcSampler), vec2(texCoord.x - 2*x, texCoord.y)).rgb;
    vec3 e = texture(sampler2D(srcTexture, srcSampler), vec2(texCoord.x,       texCoord.y)).rgb;
    vec3 f = texture(sampler2D(srcTexture, srcSampler), vec2(texCoord.x + 2*x, texCoord.y)).rgb;

    vec3 g = texture(sampler2D(srcTexture, srcSampler), vec2(texCoord.x - 2*x, texCoord.y - 2*y)).rgb;
    vec3 h = texture(sampler2D(srcTexture, srcSampler), vec2(texCoord.x,       texCoord.y - 2*y)).rgb;
    vec3 i = texture(sampler2D(srcTexture, srcSampler), vec2(texCoord.x + 2*x, texCoord.y - 2*y)).rgb;

    vec3 j = texture(sampler2D(srcTexture, srcSampler), vec2(texCoord.x - x, texCoord.y + y)).rgb;
    vec3 k = texture(sampler2D(srcTexture, srcSampler), vec2(texCoord.x + x, texCoord.y + y)).rgb;
    vec3 l = texture(sampler2D(srcTexture, srcSampler), vec2(texCoord.x - x, texCoord.y - y)).rgb;
    vec3 m = texture(sampler2D(srcTexture, srcSampler), vec2(texCoord.x + x, texCoord.y - y)).rgb;

    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
    // This shows 5 square areas that are being sampled. But some of them overlap,
    // so to have an energy preserving downsample we need to make some adjustments.
    // The weights are the distributed, so that the sum of j,k,l,m (e.g.)
    // contribute 0.5 to the final color output. The code below is written
    // to effectively yield this sum. We get:
    // 0.125*5 + 0.03125*4 + 0.0625*4 = 1
    downsample = e*0.125;
    downsample += (a+c+g+i)*0.03125;
    downsample += (b+d+f+h)*0.0625;
    downsample += (j+k+l+m)*0.125;
    out_downsample = vec4(downsample,0);
}
