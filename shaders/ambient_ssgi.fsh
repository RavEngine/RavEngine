#extension GL_EXT_samplerless_texture_functions : enable

layout(push_constant, scalar) uniform UniformBufferObject{
    uint ambientLightCount;
} ubo;

struct AmbientLightData{
    vec3 color;
    float intensity;
    uint illuminationLayers;
};

layout(binding = 0) uniform sampler g_sampler;
layout(binding = 1) uniform texture2D albedoTex;    
layout(binding = 2) uniform texture2D radianceTex;  // render layers are in the Alpha channel
layout(binding = 3) uniform texture2D giSSAO;

layout(location = 0) out vec4 outcolor;


layout(scalar, binding = 10) readonly buffer ambientLightSSBO{
    AmbientLightData ambientLights[];
};

#define RVE_SSAO 1

void main(){
    ivec2 texSize = textureSize(radianceTex,0);
    vec2 uv = gl_FragCoord.xy / texSize;

    uint entityRenderLayer = floatBitsToUint(texture(sampler2D(radianceTex,g_sampler), uv).a);

    const vec3 existingColor = texture(sampler2D(albedoTex, g_sampler), uv).rgb;

    
#if RVE_SSAO
    const float ao = texture(sampler2D(giSSAO, g_sampler), uv).a;
#else
    const float ao = 1;
#endif

    vec3 lightcolor = vec3(0);
    for(uint i = 0; i < ubo.ambientLightCount; i++){
        AmbientLightData light = ambientLights[i];
        
        if ((entityRenderLayer & light.illuminationLayers) == 0){
            continue;
        }

        lightcolor += existingColor * (light.color * light.intensity) * vec3(ao);
    }

    outcolor = vec4(lightcolor,1);
}