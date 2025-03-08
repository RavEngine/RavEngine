#include "ravengine_shader.glsl"

/*
    Adapted from code by
  (C) 2019 David Lettier
  lettier.com
*/

#define NUM_SAMPLES 8
#define NUM_NOISE   4

layout(push_constant, scalar) uniform UniformBufferObject{
    mat4 lensProjection;
    mat4 invProj;
    ivec2 screenDim;
} ubo;

layout(binding = 0) uniform sampler g_sampler;
layout(binding = 1) uniform texture2D tDepth;
layout(binding = 2) uniform texture2D tNormal;

vec3 samples[] = {
    vec3(0.049770862, -0.044709187, 0.04996342),
    vec3(0.016587738, 0.018814486, 0.002547831),
    vec3(-0.0629587, -0.030009357, 0.049461503),
    vec3(0.030610647, -0.20346682, 0.09092082),
    vec3(0.17578444, 0.18772276, 0.18102702),
    vec3(0.39494315, 0.18953449, 0.06613807),
    vec3(-0.011456773, -0.30563572, 0.37492394),
    vec3(-0.0023545611, -0.0013345107, 0.0026308813), 
};

vec3 noise[] = {
    vec3(0.49770862, -0.44709188, 0.49963418),
    vec3(0.14542674, 0.16494891, 0.022337148),
    vec3(-0.4029357, -0.19205989, 0.31655362),
    vec3(0.13510907, -0.89806044, 0.4013057),
};

layout(location = 0) out vec4 fragColor;

void main() {
  float radius    = 0.6;
  float bias      = 0.005;
  float magnitude = 1.1;
  float contrast  = 1.1;

  fragColor = vec4(1);

  vec2 texSize  = ubo.screenDim;
  vec2 texCoord = gl_FragCoord.xy / texSize;

const float depth = texture(sampler2D(tDepth, g_sampler),texCoord).r;
  if (depth >= 1) { return; }
  vec3 position = ComputeViewSpacePos(texCoord * 2 - 1, depth, ubo.invProj);

  vec3 normal = normalize(texture(sampler2D(tNormal, g_sampler), texCoord).xyz);

  int  noiseS = int(sqrt(NUM_NOISE));
  int  noiseX = int(gl_FragCoord.x - 0.5) % noiseS;
  int  noiseY = int(gl_FragCoord.y - 0.5) % noiseS;
  vec3 random = noise[noiseX + (noiseY * noiseS)];

  vec3 tangent  = normalize(random - normal * dot(random, normal));
  vec3 binormal = cross(normal, tangent);
  mat3 tbn      = mat3(tangent, binormal, normal);

  float occlusion = NUM_SAMPLES;

  for (int i = 0; i < NUM_SAMPLES; ++i) {
    vec3 samplePosition = tbn * samples[i];
         samplePosition = position.xyz + samplePosition * radius;

    vec4 offsetUV      = vec4(samplePosition, 1.0);
         offsetUV      = ubo.lensProjection * offsetUV;
         offsetUV.xyz /= offsetUV.w;
         offsetUV.xy   = offsetUV.xy * 0.5 + 0.5;

    // Config.prc
    // gl-coordinate-system  default
    // textures-auto-power-2 1
    // textures-power-2      down

    const float offsetDepth = texture(sampler2D(tDepth, g_sampler), offsetUV.xy).r;
    vec3 offsetPosition =  ComputeViewSpacePos(offsetUV.xy * 2 - 1, offsetDepth, ubo.invProj);

    float occluded = 0;
    if   (samplePosition.y + bias <= offsetPosition.y)
         { occluded = 0; }
    else { occluded = 1; }

    float intensity =
      smoothstep
        ( 0
        , 1
        ,   radius
          / abs(position.y - offsetPosition.y)
        );
    occluded  *= intensity;
    occlusion -= occluded;
  }

  occlusion /= NUM_SAMPLES;
  occlusion  = pow(occlusion, magnitude);
  occlusion  = contrast * (occlusion - 0.5) + 0.5;

  fragColor = vec4(vec3(occlusion),1);
}