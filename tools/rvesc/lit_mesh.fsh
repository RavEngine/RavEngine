struct LitOutput{
    vec4 color;
    vec3 normal;
    float roughness;
    float specular;
    float metallic;
    float ao;
};

#include "%s"

layout(location = 0) out vec4 outcolor;
layout(location = 1) out vec4 outnormal;

struct LightData{
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

    outnormal = vec4(user_out.normal,1);
}
