#extension GL_EXT_nonuniform_qualifier : enable
struct LitOutput{
    vec4 color;
    vec3 normal;
    float roughness;
    float specular;
    float metallic;
    float ao;
};

#include "%s"

#include "ravengine_shader.glsl"
#include "BRDF.glsl"

layout(location = 0) out vec4 outcolor;
layout(location = 1) out vec4 outnormal;
layout(location = 11) in vec3 worldPosition;


struct LightData{
    vec3 camPos;
    uint ambientLightCount;
    uint directionalLightCount;
};

layout(scalar, binding = 11) readonly buffer lightAuxDataSSBO{
    LightData lightConstants[];
};

struct AmbientLightData{
    vec3 color;
    float intensity;
};

layout(scalar, binding = 12) readonly buffer ambientLightSSBO{
    AmbientLightData ambientLights[];
};

struct DirectionalLightData{
    mat4 lightViewProj;
    vec3 color;
    vec3 toLight;
    float intensity;
    int castsShadows;
    int shadowmapBindlessIndex;
};

layout(scalar, binding = 13) readonly buffer dirLightSSBO{
    DirectionalLightData dirLights[];
};

layout(binding = 14) uniform sampler shadowSampler;

layout(set = 1, binding = 0) uniform texture2D shadowMaps[];      // the bindless heap must be in set 1 binding 0

void main(){

    LitOutput user_out = frag();

    // compute lighting based on the results of the user's function


    // ambient lights
    for(uint i = 0; i < lightConstants[0].ambientLightCount; i++){
        AmbientLightData light = ambientLights[i];
        outcolor += user_out.color * vec4(light.color,1) * light.intensity;
        //TODO: also factor in SSAO
    }

    // directional lights
    for(uint i = 0; i < lightConstants[0].directionalLightCount; i++){
        DirectionalLightData light = dirLights[i];
        vec3 lightResult = CalculateLightRadiance(user_out.normal, lightConstants[0].camPos, worldPosition, user_out.color.rgb, user_out.metallic, user_out.roughness, light.toLight, 1, light.color * light.intensity);
        float pcfFactor = 1;
        
        if (bool(light.castsShadows)){
             pcfFactor = pcfForShadow(worldPosition, light.lightViewProj, shadowSampler, shadowMaps[light.shadowmapBindlessIndex]);
        }

        outcolor += vec4(lightResult,1) * user_out.ao * pcfFactor;
    }

    outnormal = vec4(user_out.normal,1);
}
