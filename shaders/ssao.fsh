#include "ravengine_shader.glsl"

layout(location = 0) out float FragColor;
  

layout(push_constant, scalar) uniform UniformBufferObject{
    mat4 projection;
    mat4 invProj;
    ivec2 screenDim;
} ubo;

layout(binding = 0) uniform sampler g_sampler;
layout(binding = 1) uniform texture2D tDepth;
layout(binding = 2) uniform texture2D tNormal;

const vec3 samples[] = {
    vec3(0.060, 0.012, 0.913),
    vec3(0.286, 0.230, 0.367),
    vec3(0.945, 0.849, 0.912),
    vec3(0.188, 0.731, 0.873),
    vec3(0.610, 0.273, 0.167),
    vec3(0.295, 0.173, 0.977),
    vec3(0.575, 0.494, 0.674),
    vec3(0.794, 0.233, 0.155),
    vec3(0.747, 0.715, 0.269),
    vec3(0.985, 0.015, 0.036),
};
const float radius = 0.5;
const float bias = 0.025;

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


void main()
{
    const vec2 TexCoords = gl_FragCoord.xy / ubo.screenDim;

     // get input for SSAO algorithm
    vec2 ndc = TexCoords * 2 - 1;
    float depth = texture(sampler2D(tDepth, g_sampler),TexCoords).r;
    vec3 fragPos = ComputeViewSpacePos(ndc, depth, ubo.invProj);
    vec3 normal = normalize(texture(sampler2D(tNormal, g_sampler), TexCoords).rgb);
    vec3 randomVec = vec3(rand(TexCoords),rand(TexCoords*2),rand(TexCoords/2));
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    const float kernelSize = samples.length();
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * samples[i]; // from tangent to view-space
        samplePos = fragPos + samplePos * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = ubo.projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float normalizedDepth = texture(sampler2D(tDepth, g_sampler), offset.xy).r;
        vec3 sampleViewPos = ComputeViewSpacePos(offset.xy * 2 - 1, normalizedDepth, ubo.invProj);
        float sampleDepth = sampleViewPos.z; // get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    
    FragColor = occlusion;
}