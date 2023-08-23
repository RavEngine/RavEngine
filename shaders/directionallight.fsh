
layout(location = 0) in vec3 lightdir;
layout(location = 1) in vec4 colorintensity;
layout(location = 2) in flat vec4[4] invViewProj_elts; 


#include "lightingbindings_shared.h"
#include "ravengine_shader.glsl"

struct DirLightExtraConstants{
    mat4 lightViewProj;
};

layout(scalar, binding = 8) readonly buffer pushConstantSpill
{
	DirLightExtraConstants constants[];
};

layout(push_constant, std430) uniform UniformBufferObject{
    mat4 viewProj;
    ivec4 viewRect;     // for the whole window
    ivec4 viewRegion;   // for the virtual screen
    vec3 camPos;
    int isRenderingShadows;
} ubo;

// begin lighting functions

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

/** Parameters
 @param Normal: normal vector for this pixel
 @param camPos: location of the camera in world-space
 @param worldPos: world-space location of the pixel
 @param albedo: albedo color for the pixel
 @param metallic: metallic value for the pixel
 @param roughness: roughness value for the pixel
 @param L: normalized direction vector to the light
 @param attenuation: falloff - use 1.0 for directional lights, and ( 1.0 / (dist * dist)) for point sources, where dist is the distance from the light to the pixel
 @param lightColor: color of the light
*/
vec3 CalculateLightRadiance(vec3 Normal, vec3 camPos, vec3 WorldPos, vec3 albedo, float metallic, float roughness, vec3 L, float attenuation, vec3 lightColor){
    vec3 N = normalize(Normal);
    vec3 V = normalize(camPos - WorldPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // adapted from https://learnopengl.com/PBR/Lighting
    vec3 H = normalize(V + L);
    vec3 radiance     = lightColor * attenuation;        
    
    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, roughness);        
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;  
        
    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);                
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL; 

    return Lo;
}

// end lighting functions


void main()
{
	//calculate sampling positions using fragment pos and view dimensions
	vec2 texcoord = vec2(gl_FragCoord.x / ubo.viewRect[2], gl_FragCoord.y / ubo.viewRect[3]);

    float sampledDepthForPos = texture(sampler2D(t_depth,g_sampler), texcoord).x;
    vec2 viewTexcoord = (gl_FragCoord.xy - ubo.viewRegion.xy) / ubo.viewRegion.zw;
    mat4 invViewProj = mat4(invViewProj_elts[0],invViewProj_elts[1],invViewProj_elts[2],invViewProj_elts[3]);
    vec4 sampledPos = vec4(ComputeWorldSpacePos(viewTexcoord,sampledDepthForPos, invViewProj),1);


    // get all the PBR stuff
    vec3 albedo = texture(sampler2D(t_albedo,g_sampler), texcoord).xyz;   
    vec3 normal = texture(sampler2D(t_normal,g_sampler), texcoord).xyz;
    vec4 roughnessSpecularMetallicAO = texture(sampler2D(t_roughnessSpecularMetallicAO,g_sampler), texcoord);
    float roughness = roughnessSpecularMetallicAO.x;
    float specular = roughnessSpecularMetallicAO.y;
    float metallic = roughnessSpecularMetallicAO.z;
    float AO = roughnessSpecularMetallicAO.w;

    vec3 toLight = normalize(lightdir.xyz);

    vec3 result = CalculateLightRadiance(normal, ubo.camPos, sampledPos.xyz, albedo, metallic, roughness, toLight, 1, colorintensity.xyz * colorintensity.w);

    // is this pixel visible to the light?
    float pcfFactor = 1;
if (bool(ubo.isRenderingShadows)){
        sampledPos = constants[0].lightViewProj * sampledPos;    // where is this on the light
        sampledPos /= sampledPos.w; // perspective divide
        sampledPos.xy = sampledPos.xy * 0.5 + 0.5;    // transform to [0,1] 
        sampledPos.y = 1 - sampledPos.y;

       pcfFactor = texture(sampler2DShadow(t_depthshadow,shadowSampler), sampledPos.xyz, 0).x;
}
    
    outcolor = vec4(result * pcfFactor, 1);
	
}
