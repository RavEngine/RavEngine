
layout(location = 0) in vec3 lightdir;
layout(location = 1) in vec4 colorintensity;

layout(location = 0) out vec4 outcolor;

layout(binding = 0) uniform sampler2D s_albedo;
layout(binding = 1) uniform sampler2D s_normal;
layout(binding = 2) uniform sampler2D s_depth;
layout(binding = 3) uniform sampler2D s_depthshadow;

bool outOfRange(float f){
    return f < 0 || f > 1;
}

layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
    mat4 invViewProj;
    mat4 lightViewProj;
    ivec4 viewRect;
} ubo;

vec4 ComputeClipSpacePosition(vec2 pos, float depth){
	pos.y = 1.0 - pos.y;
	vec4 positionCS = vec4(pos * 2.0 - 1.0, depth, 1);
	return positionCS;
}

vec3 ComputeWorldSpacePos(vec2 positionNDC, float depth, mat4 invViewProj){
	vec4 positionCS = ComputeClipSpacePosition(positionNDC, depth);
	vec4 hpositionWS = invViewProj * positionCS;
	return (hpositionWS / hpositionWS.w).xyz;
}

void main()
{
	//calculate sampling positions using fragment pos and view dimensions
	vec2 texcoord = vec2(gl_FragCoord.x / ubo.viewRect[2], gl_FragCoord.y / ubo.viewRect[3]);
    
    // is this pixel visible to the light? if not, discard

    vec3 normal = texture(s_normal, texcoord).xyz;
    vec3 toLight = normalize(lightdir.xyz);

    int enabled = 1;
    const float bias = 0.005; //TODO: calcuate as a function of the difference between the normal and the light dir
#if 1
    //TODO: check if shadow is enabled
        float sampledDepthForPos = texture(s_depth, texcoord).x;
        vec4 sampledPos = vec4(ComputeWorldSpacePos(texcoord,sampledDepthForPos, ubo.invViewProj),1);
        sampledPos = ubo.lightViewProj * sampledPos;    // where is this on the light
        sampledPos /= sampledPos.w; // perspective divide
        sampledPos.xy = sampledPos.xy * 0.5 + 0.5;    // transform to [0,1] 
        sampledPos.z *= -1;
        
        float sampledDepth = (outOfRange(sampledPos.x) || outOfRange(sampledPos.y)) ? 1 : texture(s_depthshadow, sampledPos.xy).x;

        // in shadow
        if (sampledDepth.x < sampledPos.z - bias){
            enabled = 0;
        }
#endif
    
    float intensity = colorintensity[3];
    
    vec3 albedo = texture(s_albedo, texcoord).xyz;   
    
    float nDotL = max(dot(normal, toLight), 0);
    
    vec3 diffuseLight = albedo * nDotL;
    outcolor = vec4(intensity * colorintensity.xyz * diffuseLight * enabled, enabled);
	
}
