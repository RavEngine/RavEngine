#extension GL_EXT_samplerless_texture_functions: enable
#include "ravengine_shader.glsl"

// Adapted from: https://cybereality.com/screen-space-indirect-lighting-with-visibility-bitmask-improvement-to-gtao-ssao-real-time-ambient-occlusion-algorithm-glsl-shader-implementation/

// Adapted from "Screen Space Indirect Lighting with Visibility Bitmask" by Olivier Therrien, et al.
// https://cdrinmatane.github.io/posts/cgspotlight-slides/
precision highp float;
precision highp sampler2D;

layout(location = 0) out vec4 fragColor;

layout(push_constant, scalar) uniform UniformBufferObject{
    mat4 projection;
    mat4 invProj;
    ivec2 outputSize;
	float sampleCount;
    float sampleRadius;
    float sliceCount;
    float hitThickness;
} ubo;

layout(binding = 0) uniform sampler g_sampler;
layout(binding = 1) uniform texture2D screenDepth;
layout(binding = 2) uniform texture2D screenNormal;
layout(binding = 3) uniform texture2D screenLight;

const float pi = 3.14159265359;
const float twoPi = 2.0 * pi;
const float halfPi = 0.5 * pi;

// https://blog.demofox.org/2022/01/01/interleaved-gradient-noise-a-different-kind-of-low-discrepancy-sequence/
float randf(int x, int y) {
    return mod(52.9829189 * mod(0.06711056 * float(x) + 0.00583715 * float(y), 1.0), 1.0);
}

// https://cdrinmatane.github.io/posts/ssaovb-code/
const uint sectorCount = 32u;
uint updateSectors(float minHorizon, float maxHorizon, uint outBitfield) {
    uint startBit = uint(minHorizon * float(sectorCount));
    uint horizonAngle = uint(ceil((maxHorizon - minHorizon) * float(sectorCount)));
    uint angleBit = horizonAngle > 0u ? uint(0xFFFFFFFFu >> (sectorCount - horizonAngle)) : 0u;
    uint currentBitfield = angleBit << startBit;
    return outBitfield | currentBitfield;
}

// get indirect lighting and ambient occlusion
vec4 getVisibility() {
    ivec2 screenSize = ubo.outputSize;

    vec2 fragUV = gl_FragCoord.xy / vec2(screenSize.x, screenSize.y);

    uint indirect = 0u;
    uint occlusion = 0u;

    float visibility = 0.0;
    vec3 lighting = vec3(0.0);
    vec2 frontBackHorizon = vec2(0.0);
    vec2 aspect = screenSize.yx / screenSize.x;
    const float depth = texture(sampler2D(screenDepth, g_sampler), fragUV).r;
    vec3 position = ComputeViewSpacePos(fragUV,depth,ubo.invProj);
    vec3 camera = normalize(-position);
    vec3 normal = normalize(texture(sampler2D(screenNormal, g_sampler), fragUV).rgb);

    float sliceRotation = twoPi / (ubo.sliceCount - 1.0);
    float sampleScale = (-ubo.sampleRadius * ubo.projection[0][0]) / position.z;
    float sampleOffset = 0.01;
    float jitter = randf(int(gl_FragCoord.x), int(gl_FragCoord.y)) - 0.5;

    for (float slice = 0.0; slice < ubo.sliceCount + 0.5; slice += 1.0) {
        float phi = sliceRotation * (slice + jitter) + pi;
        vec2 omega = vec2(cos(phi), sin(phi));
        vec3 direction = vec3(omega.x, omega.y, 0.0);
        vec3 orthoDirection = direction - dot(direction, camera) * camera;
        vec3 axis = cross(direction, camera);
        vec3 projNormal = normal - axis * dot(normal, axis);
        float projLength = length(projNormal);

        float signN = sign(dot(orthoDirection, projNormal));
        float cosN = clamp(dot(projNormal, camera) / projLength, 0.0, 1.0);
        float n = signN * acos(cosN);

        for (float currentSample = 0.0; currentSample < ubo.sampleCount + 0.5; currentSample += 1.0) {
            float sampleStep = (currentSample + jitter) / ubo.sampleCount + sampleOffset;
            vec2 sampleUV = fragUV - sampleStep * sampleScale * omega * aspect;

            const float sampleDepth = texture(sampler2D(screenDepth, g_sampler), sampleUV).r;

            vec3 samplePosition = ComputeViewSpacePos(sampleUV, sampleDepth, ubo.invProj);
            vec3 sampleNormal = normalize(texture(sampler2D(screenNormal, g_sampler), sampleUV).rgb);
            vec3 sampleLight = texture(sampler2D(screenLight, g_sampler), sampleUV).rgb;
            vec3 sampleDistance = samplePosition - position;
            float sampleLength = length(sampleDistance);
            vec3 sampleHorizon = sampleDistance / sampleLength;

            frontBackHorizon.x = dot(sampleHorizon, camera);
            frontBackHorizon.y = dot(normalize(sampleDistance - camera * ubo.hitThickness), camera);

            frontBackHorizon = acos(frontBackHorizon);
            frontBackHorizon = clamp((frontBackHorizon + n + halfPi) / pi, 0.0, 1.0);

            indirect = updateSectors(frontBackHorizon.x, frontBackHorizon.y, 0u);
            lighting += (1.0 - float(bitCount(indirect & ~occlusion)) / float(sectorCount)) *
                sampleLight * clamp(dot(normal, sampleHorizon), 0.0, 1.0) *
                clamp(dot(sampleNormal, -sampleHorizon), 0.0, 1.0);
            occlusion |= indirect;
        }
        visibility += 1.0 - float(bitCount(occlusion)) / float(sectorCount);
    }

    visibility /= ubo.sliceCount;
    lighting /= ubo.sliceCount;

    return vec4(lighting, min(1,visibility));   // pin AO to 1
}

