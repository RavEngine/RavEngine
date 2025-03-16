#extension GL_EXT_samplerless_texture_functions : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(push_constant, scalar) uniform UniformBufferObject{
    mat4 invView;
    uint ambientLightCount;
    float ssaoStrength;
    uint options;
} ubo;

struct AmbientLightData{
    vec3 color;
    float intensity;
    uint illuminationLayers;
    uint environmentCubemapBindlessIndex;
    uint irradianceCubemapBindlessIndex;
};

layout(binding = 0) uniform sampler g_sampler;
layout(binding = 1) uniform texture2D albedoTex;    
layout(binding = 2) uniform texture2D radianceTex;  // render layers are in the Alpha channel
layout(binding = 3) uniform texture2D giSSAO;
layout(binding = 4) uniform texture2D normalTex;

layout(set = 2, binding = 0) uniform textureCube cubeMaps[];


layout(location = 0) out vec4 outcolor;


layout(scalar, binding = 10) readonly buffer ambientLightSSBO{
    AmbientLightData ambientLights[];
};

#define RVE_SSAO 1

void main(){
    bool ssaoEnabled = bool(ubo.options & (1));
    bool ssgiEnabled = bool(ubo.options & (1 << 1));

    ivec2 texSize = textureSize(radianceTex,0);
    vec2 uv = gl_FragCoord.xy / texSize;

    uint entityRenderLayer = floatBitsToUint(texture(sampler2D(radianceTex,g_sampler), uv).a);

    const vec3 albedo = texture(sampler2D(albedoTex, g_sampler), uv).rgb;
    vec3 worldNormal = texture(sampler2D(normalTex, g_sampler), uv).rgb;
    worldNormal = mat3(ubo.invView) * worldNormal;  // the texture is in view space

    vec4 giao = texture(sampler2D(giSSAO, g_sampler), uv);

#if RVE_SSAO
    float ao = ssaoEnabled ? giao.a : 1;
#else
    float ao = 1;
#endif

    if (ssaoEnabled){
        ao = pow(ao,ubo.ssaoStrength);
    }

    // ambient lights and AO
    vec3 ambientcontrib = vec3(0);
    for(uint i = 0; i < ubo.ambientLightCount; i++){
        AmbientLightData light = ambientLights[i];
        
        if ((entityRenderLayer & light.illuminationLayers) == 0){
            continue;
        }

        vec3 environmentColor = vec3(1);
        if (light.environmentCubemapBindlessIndex != (~0)){
            // use cubemap as environment color
            vec3 incident = normalize(vec3(uv * 2 - 1,-1)); // view space
            incident = mat3(ubo.invView) * incident;
            vec3 refl = reflect(incident, worldNormal);

            environmentColor = texture(samplerCube(cubeMaps[light.environmentCubemapBindlessIndex], g_sampler), refl).rgb;
        }

        ambientcontrib += albedo * (light.color * light.intensity * environmentColor) * vec3(ao);
    }

    // Global illumination
    vec3 gicontrib =  ssgiEnabled ? (albedo * giao.rgb) : vec3(0);

    outcolor = vec4(ambientcontrib + gicontrib,1);
}