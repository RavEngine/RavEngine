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
    vec3(0.049770862, -0.044709187, 0.04996342),
    vec3(0.016587738, 0.018814486, 0.002547831),
    vec3(-0.0629587, -0.030009357, 0.049461503),
    vec3(0.030610647, -0.20346682, 0.09092082),
    vec3(0.17578444, 0.18772276, 0.18102702),
    vec3(0.39494315, 0.18953449, 0.06613807),
    vec3(-0.011456773, -0.30563572, 0.37492394),
    vec3(-0.0023545611, -0.0013345107, 0.0026308813), 
};
const float kernelSize = samples.length();

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
    const float depth = texture(sampler2D(tDepth, g_sampler),TexCoords).r;
    const vec3 fragPos = ComputeViewSpacePos(TexCoords, depth, ubo.invProj);
    const vec3 normal = normalize(texture(sampler2D(tNormal, g_sampler), TexCoords).rgb);
    const vec3 randomVec = vec3(rand(TexCoords) * 2 - 1,rand(TexCoords*2) * 2 - 1, 0);        // corresponds to the "noiseTexture" in the original. Z is 0 intentionally
    // create TBN change-of-basis matrix: from tangent-space to view-space
    const vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    const vec3 bitangent = cross(normal, tangent);
    const mat3 TBN = mat3(tangent, bitangent, normal);
    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        const vec3 currentSample = samples[i];
        vec3 samplePos = TBN * currentSample; // from tangent to view-space
        samplePos = fragPos + samplePos * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = ubo.projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        const float normalizedDepth = texture(sampler2D(tDepth, g_sampler), offset.xy).r;
        const vec3 sampleViewPos = ComputeViewSpacePos(offset.xy, normalizedDepth, ubo.invProj);
        const float sampleDepth = sampleViewPos.z; // get depth value of kernel sample
        
        // range check & accumulate
        const float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        float sampleCheck = sampleDepth >= samplePos.z + bias ? 1.0 : 0.0;
        occlusion += sampleCheck * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    
    FragColor = occlusion;
}