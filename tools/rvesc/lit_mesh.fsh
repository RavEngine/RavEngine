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

    layout(location = 0) out vec4 outcolor;     // accumulation if in transparent mode
    layout(location = 1) out vec4 outnormal;
#if RVE_TRANSPARENT
    layout(location = 2) out float revealage;

    // adapted from: https://casual-effects.blogspot.com/2015/03/implemented-weighted-blended-order.html
    void writePixel(vec4 premultipliedReflect, vec3 transmit, float csZ) { 
    /* Modulate the net coverage for composition by the transmission. This does not affect the color channels of the
       transparent surface because the caller's BSDF model should have already taken into account if transmission modulates
       reflection. This model doesn't handled colored transmission, so it averages the color channels. See 

          McGuire and Enderton, Colored Stochastic Shadow Maps, ACM I3D, February 2011
          http://graphics.cs.williams.edu/papers/CSSM/

       for a full explanation and derivation.*/

    premultipliedReflect.a *= 1.0 - clamp((transmit.r + transmit.g + transmit.b) * (1.0 / 3.0), 0, 1);

    /* You may need to adjust the w function if you have a very large or very small view volume; see the paper and
       presentation slides at http://jcgt.org/published/0002/02/09/ */
    // Intermediate terms to be cubed
    float a = min(1.0, premultipliedReflect.a) * 8.0 + 0.01;
    float b = -gl_FragCoord.z * 0.95 + 1.0;

    /* If your scene has a lot of content very close to the far plane,
       then include this line (one rsqrt instruction):
       b /= sqrt(1e4 * abs(csZ)); */
    float w    = clamp(a * a * a * 1e8 * b * b * b, 1e-2, 3e2);
    outcolor     = premultipliedReflect * w;
    revealage = premultipliedReflect.a;
}
#else
   
#endif
layout(location = 11) in vec3 worldPosition;
layout(location = 12) in vec3 viewPosition;

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

layout(scalar, binding = 17) readonly buffer spotLightSSBO{
    SpotLight spotLights[];
};

layout(set = 1, binding = 0) uniform texture2D shadowMaps[];      // the bindless heap must be in set 1 binding 0

void main(){

    LitOutput user_out = frag();
    outcolor = vec4(0); // NV: these don't default-init to 0
    outnormal = vec4(user_out.normal,1);

    // compute lighting based on the results of the user's function

    // ambient lights
    for(uint i = 0; i < engineConstants[0].ambientLightCount; i++){
        AmbientLightData light = ambientLights[i];

        float ao = 1;
        #if 0
        if (ubo.ssao_enabled > 0){
		    ao = texture(sampler2D(t_ssao, g_sampler), texcoord).r;
	    }
        #endif

        outcolor += user_out.color * vec4(light.color,1) * light.intensity * ao;
        
    }

    // directional lights
    for(uint i = 0; i < engineConstants[0].directionalLightCount; i++){
        DirectionalLightData light = dirLights[i];
        vec3 lightResult = CalculateLightRadiance(user_out.normal, engineConstants[0].camPos, worldPosition, user_out.color.rgb, user_out.metallic, user_out.roughness, light.toLight, 1, light.color * light.intensity);
        float pcfFactor = 1;
        
        if (bool(light.castsShadows)){
             pcfFactor = pcfForShadow(worldPosition, light.lightViewProj, shadowSampler, shadowMaps[light.shadowmapBindlessIndex]);
        }

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

        vec3 toLight = normalize(light.position - worldPosition);

        float dist = distance(worldPosition, light.position);

        vec3 result = CalculateLightRadiance(user_out.normal, engineConstants[0].camPos, worldPosition, user_out.color.rgb, user_out.metallic, user_out.roughness, toLight, getLightAttenuation(dist),  light.color * light.intensity);
        float pcfFactor = 1;

        if (bool(light.castsShadows)){
            // adapted from: https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/Mesh.hlsl#L233
            vec3 shadowPos = worldPosition - light.position;
            float shadowDistance = length(shadowPos);
            vec3 shadowDir = normalize(shadowPos);

            float projectedDistance  = max(max(abs(shadowPos.x), abs(shadowPos.y)), abs(shadowPos.z));

            float nearClip = engineConstants[0].zNear;
            float a = 0.0;
            float b = nearClip;
            float z = projectedDistance * a + b;
            float dbDistance = z / projectedDistance;

            //pcfFactor = texture(samplerCubeShadow(shadowMaps[light.shadowmapBindlessIndex], shadowSampler), vec4(shadowDir, dbDistance)).r;
        }

        outcolor += vec4(result * user_out.ao * pcfFactor,0);
    }

    // spot lights
    const uint nSpotLights = clusters[tileIndex].spotLightCount;
    for(uint i = 0; i < nSpotLights && i < CLUSTER_MAX_SPOTS; i++){
        uint lightIndex = clusters[tileIndex].spotLightIndices[i];
        SpotLight light = spotLights[lightIndex];

        vec4 position = light.worldTransform * vec4(0,0,0,1);

        vec3 toLight = normalize(position.xyz - worldPosition);

	    float dist = distance(worldPosition, position.xyz);

        // is the pixel inside the light area?
        float coneDotFactor = cos(radians(light.coneAngle));
        mat3 rotScaleOnly = mat3(light.worldTransform);
        vec3 forward = normalize(rotScaleOnly * vec3(0, -1, 0));    // spot lights point down by default

	    float pixelAngle = dot(-forward,toLight);   

        vec3 result = CalculateLightRadiance(user_out.normal, engineConstants[0].camPos, worldPosition, user_out.color.rgb, user_out.metallic, user_out.roughness, toLight, getLightAttenuation(dist),  light.color * light.intensity);

        float pcfFactor = 1;
        if (bool(light.castsShadows)){
            pcfFactor = pcfForShadow(worldPosition, light.lightViewProj, shadowSampler, shadowMaps[light.shadowmapBindlessIndex]);
        }

        pcfFactor = pcfFactor * (int(pixelAngle > coneDotFactor));

        outcolor += vec4(result * user_out.ao * pcfFactor, 0);
    }

    #if RVE_TRANSPARENT
        writePixel(outcolor, vec3(0,0,0),1.0);  //TODO: fix parameters
    #endif

}
