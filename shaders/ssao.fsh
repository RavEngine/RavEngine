
#include "ravengine_shader.glsl"

// adapted from: https://learnopengl.com/Advanced-Lighting/SSAO

layout(push_constant, std430) uniform UniformBufferObject{
    mat4 viewProj;
    ivec4 viewRect;     // for the whole screen
    ivec4 viewRegion;   // for the virtual screen
} ubo;

layout(binding = 0) uniform sampler srcSampler;
layout(binding = 1) uniform texture2D normalTex;
layout(binding = 2) uniform texture2D depthTex;

struct ssaoPushSpill {
    mat4 projOnly;
    mat4 invViewProj;
};

layout(scalar, binding = 7) readonly buffer ssaoConstantsSpill{
    ssaoPushSpill constants[];
};

layout(scalar, binding = 8) readonly buffer ssaoSampleBuffer{
    vec3 samples[];
};

layout(location = 0) out vec4 outcolor;

int kernelSize = 8;    // also the number of samples
float radius = 0.5;
float bias = 0.025;

float rand(in vec2 ip) {
    const float seed = 12345678;
    uvec2 p = uvec2(floatBitsToUint(ip.x), floatBitsToUint(ip.y));
    uint s = floatBitsToUint(seed);
    s ^= (s ^ ((~p.x << s) + (~p.y << s)));
    
    p ^= (p << 17U);
    p ^= ((p ^ s) >> 13U);
    p ^= (p << 5U);
    p ^= ((s + (s&(p.x^p.y))) >> 3U);
    
    uint n = (p.x*s+p.y)+((p.x ^ p.y) << ~p.x ^ s) + ((p.x ^ p.y) << ~p.y ^ s);
    return float(n*50323U) / float(0xFFFFFFFFU);
}

void main(){

    // get input for SSAO algorithm
    vec2 texcoord = vec2(gl_FragCoord.x / ubo.viewRect[2], gl_FragCoord.y / ubo.viewRect[3]);
    float sampledDepthForPos = texture(sampler2D(depthTex, srcSampler), texcoord).r;
    vec2 viewTexcoord = (gl_FragCoord.xy - ubo.viewRegion.xy) / ubo.viewRegion.zw;

    mat4 invViewProj = constants[0].invViewProj;

    vec3 fragPos = ComputeWorldSpacePos(viewTexcoord, sampledDepthForPos, invViewProj);
    vec3 normal = normalize(texture(sampler2D(normalTex, srcSampler), texcoord).rgb);
    vec3 randomVec = normalize(vec3(rand(texcoord), rand(texcoord * 2), rand(texcoord * 0.5)));

    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * samples[i]; // from tangent to view-space
        samplePos = fragPos + samplePos * radius; 

        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = constants[0].projOnly * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sampleDepth = texture(sampler2D(depthTex, srcSampler), offset.xy).r; // get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;     
    }
    occlusion = 1.0 - (occlusion / kernelSize);

    outcolor = vec4(occlusion,1,1,1);
}
