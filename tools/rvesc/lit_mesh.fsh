#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_shader_16bit_storage : enable
#extension GL_EXT_shader_explicit_arithmetic_types : enable

#if !RVE_DEPTHONLY
	#ifdef RVE_LIGHTMAP_UV
        #define RVE_HAS_LIGHTMAP_UV 1
	#endif
#endif


struct LitOutput{
    vec4 color;
    vec3 normal;
    vec3 emissiveColor;
    float roughness;
    float specular;
    float metallic;
    float ao;
};

struct EnvironmentData{
    mat4 entityModelMtx;
    mat3 TBN;
};

#include "%s"

#include "ravengine_shader.glsl"
#include "BRDF.glsl"
#include "cluster_shared.glsl"

#include "lit_mesh_shared.glsl"
#include "mesh_shared.glsl"

#include "enginedata.glsl"
#include "make_engine_data.glsl"

#if !RVE_DEPTHONLY && !RVE_TRANSPARENT
#define RVE_EXTRAOUTPUT 1
#else
#define RVE_EXTRAOUTPUT 0
#endif

#if RVE_PREPASS
layout(location = 0) out vec4 outViewSpaceNormal;
#endif

#if RVE_EXTRAOUTPUT
layout(location = 1) out vec4 outRadiance;
layout(location = 2) out vec4 outAlbedo;
#endif

struct AmbientLightData{
    vec3 color;
    float intensity;
    uint illuminationLayers;
    uint environmentCubemapBindlessIndex;
};

layout(scalar, binding = 12) readonly buffer ambientLightSSBO{
    AmbientLightData ambientLights[];
};

struct DirectionalLightData{
    vec3 color;
    vec3 toLight;
    float intensity;
    int castsShadows;
    int shadowmapBindlessIndex[SH_MAX_CASCADES];
    float cascadeDistances[SH_MAX_CASCADES];
    uint shadowRenderLayers;
    uint illuminationLayers;
    uint numCascades;
};

struct DirectionalLightDataPassVarying{
    mat4 lightViewProj[SH_MAX_CASCADES];
};

layout(scalar, binding = 13) readonly buffer dirLightSSBO{
    DirectionalLightData dirLights[];
};


layout(scalar, binding = 15) readonly buffer pointLightSSBO{
    PointLight pointLights[];
};

layout(scalar, binding = 16) readonly buffer clusterSSBO{
    Cluster clusters[];
};

layout(scalar, binding = 17) readonly buffer spotLightSSBO{
    SpotLight spotLights[];
};

layout(scalar, binding = 28) readonly buffer renderLayerSSBO{
    uint entityRenderLayers[];
};

layout(scalar, binding = 29) readonly buffer perObjectAttributesSSBO{
    uint16_t perObjectFlags[];
};

layout(scalar, binding = 30) readonly buffer dirLightVaryingSSBO{
    DirectionalLightDataPassVarying dirLightsVarying[];
};

#if !RVE_DEPTHONLY  // depth only calculation does not do shadow reads
layout(binding = 14) uniform sampler shadowSampler;
layout(binding = 31) uniform sampler environmentSampler;
layout(set = 1, binding = 0) uniform texture2D shadowMaps[];      // the bindless heap must be in set 1 binding 0
layout(set = 2, binding = 0) uniform textureCube cubeMaps[];    // we alias these because everything goes into the one heap
#endif

void main(){

    // normals are in local space
    // but we need them in world space
    mat4 entityModelMtx = model[varyingEntityID];

    const mat3 TBN = mat3(inTBN[0],inTBN[1],inTBN[2]);

    EnvironmentData envData;
    envData.entityModelMtx = entityModelMtx;
    envData.TBN = TBN;

    LitOutput user_out = frag(envData);

    const vec3 objectSpaceNormal = TBN * user_out.normal;


    const vec3 worldNormal = mat3(entityModelMtx) * objectSpaceNormal;
    
     // this part needs them in view space
    EngineData data = make_engine_data(engineConstants[0]);

    #if RVE_EXTRAOUTPUT
        outAlbedo = user_out.color;
    #endif
    #if RVE_PREPASS
    outViewSpaceNormal = vec4(mat3(data.viewOnly) * worldNormal,1);
    #endif

    vec4 outcolor = vec4(0); // NV: these don't default-init to 0

    vec3 radiance = vec3(0);
    

#if !RVE_DEPTHONLY

    const uint entityRenderLayer = entityRenderLayers[varyingEntityID];
    const uint16_t attributeBitmask = perObjectFlags[varyingEntityID];
    const bool recievesShadows = bool(attributeBitmask & (1 << 3));

    // compute lighting based on the results of the user's function

    // ambient lights
    for(uint i = 0; i < engineConstants[0].ambientLightCount; i++){
        AmbientLightData light = ambientLights[i];
        
        if ((entityRenderLayer & light.illuminationLayers) == 0){
            continue;
        }

        vec3 environmentColor = vec3(1);
        if (light.environmentCubemapBindlessIndex != (~0)){
            // use cubemap as environment color
            environmentColor = texture(samplerCube(cubeMaps[light.environmentCubemapBindlessIndex], environmentSampler), worldNormal).rgb;
        }

        outcolor += user_out.color * vec4(light.color * light.intensity * environmentColor,1);
    }

    // directional lights
    for(uint i = 0; i < engineConstants[0].directionalLightCount; i++){
        DirectionalLightData light = dirLights[i];
        
        if ((entityRenderLayer & light.illuminationLayers) == 0){
            continue;
        }
        
        vec3 rad;
        vec3 lightResult = CalculateLightRadiance(worldNormal, engineConstants[0].camPos, worldPosition, user_out.color.rgb, user_out.metallic, user_out.roughness, light.toLight, 1, light.color * light.intensity, rad);
        float pcfFactor = 1;
        
        vec4 color = vec4(0);
        
        vec4 pallete[] = {
            vec4(1,0,0,1),
            vec4(0,1,0,1),
            vec4(0,0,1,1),
            vec4(1,0,1,1),
        };
        
        if (recievesShadows && bool(light.castsShadows)){
            DirectionalLightDataPassVarying passVarying = dirLightsVarying[i];
            vec4 viewSpace = engineConstants[0].viewOnly * vec4(worldPosition,1);
            float depthValue = abs(viewSpace.z);
            int cascadeCount = int(light.numCascades);
            
            int layer = -1;
            for (int i = 0; i < cascadeCount; ++i)
            {
                if (depthValue < light.cascadeDistances[i])
                {
                    layer = i;
                    break;
                }
            }
            if (layer == -1)
            {
                layer = cascadeCount - 1;
            }
            
            //layer = cascadeCount - layer - 1;
            
             pcfFactor = pcfForShadow(worldPosition, passVarying.lightViewProj[layer], shadowSampler, shadowMaps[light.shadowmapBindlessIndex[layer]]);
             radiance += rad * pcfFactor;
            color = pallete[layer];
        }
        //outcolor += color;

        outcolor += vec4(lightResult * user_out.ao * pcfFactor,0);
    }

    // Locating which cluster this fragment is part of
    // adpated from: https://github.com/DaveH355/clustered-shading
    const uint zTile = uint((log(abs(viewPosition.z) / engineConstants[0].zFar) * engineConstants[0].gridSize.z) / log(engineConstants[0].zNear / engineConstants[0].zFar));
    const vec2 tileSize = engineConstants[0].screenDimensions.zw / engineConstants[0].gridSize.xy;

    vec2 virtualScreenCoord = (gl_FragCoord.xy - engineConstants[0].screenDimensions.xy);

    const uvec3 tile = uvec3(virtualScreenCoord / tileSize, zTile);
    uint tileIndex = tile.x + (tile.y * engineConstants[0].gridSize.x) + (tile.z * engineConstants[0].gridSize.x * engineConstants[0].gridSize.y);

    //outcolor = vec4(tile, 1);

    const uint nPointLights = clusters[tileIndex].pointLightCount;
    
    // point lights
    for(uint i = 0; i < nPointLights && i < CLUSTER_MAX_POINTS; i++){
        uint lightIndex = clusters[tileIndex].pointLightIndices[i];
        PointLight light = pointLights[lightIndex];
        
        if ((entityRenderLayer & light.illuminationLayers) == 0){
            continue;
        }

        vec3 toLight = normalize(light.position - worldPosition);

        float dist = distance(worldPosition, light.position);

        vec3 rad;
        vec3 result = CalculateLightRadiance(worldNormal, engineConstants[0].camPos, worldPosition, user_out.color.rgb, user_out.metallic, user_out.roughness, toLight, getLightAttenuation(dist),  light.color * light.intensity, rad);
        float pcfFactor = 1;

        if (recievesShadows && bool(light.castsShadows)){

            vec3 pallete[] = {
                vec3(1,0,0),
                vec3(1,0,0),
                
                vec3(0,1,0),
                vec3(0,1,0),

                vec3(0,0,1),
                vec3(0,0,1),
            };
            vec3 dbgcolor = vec3(0);
            for(uint i = 0; i < light.shadowmapBindlessIndices.length(); i++){
                mat4 lightCenteredView = light.viewMats[i];

                mat4 faceViewProj = light.projMat * lightCenteredView;

                pcfFactor = min(pcfForShadow(worldPosition, faceViewProj, shadowSampler, shadowMaps[light.shadowmapBindlessIndices[i]]), pcfFactor);
                dbgcolor += pcfFactor * pallete[i];
            }
            //outcolor *= vec4(dbgcolor,1);
            radiance += rad * pcfFactor;
        }

        outcolor += vec4(result * user_out.ao * pcfFactor,0);
    }

    // spot lights
    const uint nSpotLights = clusters[tileIndex].spotLightCount;
    for(uint i = 0; i < nSpotLights && i < CLUSTER_MAX_SPOTS; i++){
        uint lightIndex = clusters[tileIndex].spotLightIndices[i];
        SpotLight light = spotLights[lightIndex];
        
        if ((entityRenderLayer & light.illuminationLayers) == 0){
            continue;
        }

        vec4 position = light.worldTransform * vec4(0,0,0,1);

        vec3 toLight = normalize(position.xyz - worldPosition);

	    float dist = distance(worldPosition, position.xyz);

        // is the pixel inside the light area?
        float coneDotFactor = cos(radians(light.coneAngle));
        mat3 rotScaleOnly = mat3(light.worldTransform);
        vec3 forward = normalize(rotScaleOnly * vec3(0, -1, 0));    // spot lights point down by default

	    float pixelAngle = dot(-forward,toLight);   

        vec3 rad;
        vec3 result = CalculateLightRadiance(worldNormal, engineConstants[0].camPos, worldPosition, user_out.color.rgb, user_out.metallic, user_out.roughness, toLight, getLightAttenuation(dist),  light.color * light.intensity, rad);
        float pcfFactor = 1;
        if (recievesShadows && bool(light.castsShadows)){
            pcfFactor = pcfForShadow(worldPosition, light.lightViewProj, shadowSampler, shadowMaps[light.shadowmapBindlessIndex]);
            radiance += rad * pcfFactor;
        }

        pcfFactor = pcfFactor * (int(pixelAngle > coneDotFactor));

        outcolor += vec4(result * user_out.ao * pcfFactor, 0);
    }

    // add the emissive component
    outcolor += vec4(user_out.emissiveColor,0);  // don't want to add emissivity to the alpha channel

        #if RVE_TRANSPARENT
            beginInvocationInterlockARB();
            writeTransparency(outcolor);
            endInvocationInterlockARB();
        #else
            result = outcolor;
        #endif

#endif // RVE_DEPTHONLY
    #if RVE_EXTRAOUTPUT
    outRadiance = vec4(radiance,uintBitsToFloat(entityRenderLayer));
    #endif
}
