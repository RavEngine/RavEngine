#extension GL_EXT_samplerless_texture_functions : enable
// adapted from: https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom

layout(push_constant, std430) uniform UniformBufferObject{
    ivec4 targetDim;
    float bloomStrength;
} ubo;

layout(binding = 0) uniform texture2D srcTexture;
layout(binding = 1) uniform texture2D bloomTexture;
layout(binding = 2) uniform sampler srcSampler;

layout(location = 0) out vec4 outcolor;

void main(){
    vec2 srcResolution = textureSize(srcTexture,0).xy;
    vec2 uv = gl_FragCoord.xy / srcResolution;

    vec3 color = texture(sampler2D(srcTexture, srcSampler), uv).rgb;
    vec3 bloom = texture(sampler2D(bloomTexture, srcSampler), uv).rgb;
    vec3 result = mix(color, bloom, ubo.bloomStrength);
    outcolor = vec4(result, 1);
}
