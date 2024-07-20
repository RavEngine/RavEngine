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
#include "cluster_shared.glsl"

layout(location = 0) out vec4 outcolor;
layout(location = 1) out vec4 outnormal;
layout(location = 11) in vec3 worldPosition;

#include "lit_mesh_shared.glsl"

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

layout(scalar, binding = 15) readonly buffer pointLightSSBO{
    PointLight pointLights[];
};

layout(scalar, binding = 16) readonly buffer clusterSSBO{
    Cluster clusters[];
};

layout(set = 1, binding = 0) uniform texture2D shadowMaps[];      // the bindless heap must be in set 1 binding 0

void main(){

    LitOutput user_out = frag();

    // compute lighting based on the results of the user's function


    // ambient lights
    for(uint i = 0; i < engineConstants[0].ambientLightCount; i++){
        AmbientLightData light = ambientLights[i];
        outcolor += user_out.color * vec4(light.color,1) * light.intensity;
        //TODO: also factor in SSAO
    }

    // directional lights
    for(uint i = 0; i < engineConstants[0].directionalLightCount; i++){
        DirectionalLightData light = dirLights[i];
        vec3 lightResult = CalculateLightRadiance(user_out.normal, engineConstants[0].camPos, worldPosition, user_out.color.rgb, user_out.metallic, user_out.roughness, light.toLight, 1, light.color * light.intensity);
        float pcfFactor = 1;
        
        if (bool(light.castsShadows)){
             pcfFactor = pcfForShadow(worldPosition, light.lightViewProj, shadowSampler, shadowMaps[light.shadowmapBindlessIndex]);
        }

        outcolor += vec4(lightResult * user_out.ao * pcfFactor,1);
    }

    // point lights
    const vec3 viewPosition = (engineConstants[0].viewOnly * vec4(worldPosition,1)).xyz;  //TODO: compute this in the vertex stage

    // Locating which cluster this fragment is part of
    const uint zTile = uint((log(abs(viewPosition.z) / engineConstants[0].zNear) * engineConstants[0].gridSize.z) / log(engineConstants[0].zFar / engineConstants[0].zNear));
    const vec2 tileSize = engineConstants[0].screenDimensions / engineConstants[0].gridSize.xy;
    const uvec3 tile = uvec3(gl_FragCoord.xy / tileSize, zTile);
    const uint tileIndex =
        tile.x + (tile.y * engineConstants[0].gridSize.x) + (tile.z * engineConstants[0].gridSize.x * engineConstants[0].gridSize.y);

    const uint lightCount = clusters[tileIndex].count;
    
    for(uint i = 0; i < lightCount; i++){
        uint lightIndex = clusters[tileIndex].lightIndices[i];
        PointLight light = pointLights[lightIndex];

        vec3 toLight = normalize(light.position - worldPosition);

        float dist = distance(worldPosition, light.position);

        vec3 result = CalculateLightRadiance(user_out.normal, engineConstants[0].camPos, worldPosition, user_out.color.rgb, user_out.metallic, user_out.roughness, toLight, 1.0 / (dist * dist),  light.color * light.intensity);
        float pcfFactor = 1;
        outcolor += vec4(result * user_out.ao * pcfFactor,1);
    }

    outnormal = vec4(user_out.normal,1);
}
