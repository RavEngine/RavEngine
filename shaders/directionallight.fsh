
layout(location = 0) in vec3 lightdir;
layout(location = 1) in vec4 colorintensity;

layout(location = 0) out vec4 outcolor;

layout(binding = 0) uniform sampler2D s_albedo;
layout(binding = 1) uniform sampler2D s_normal;

//SAMPLER2D(s_pos,2);
//SAMPLER2D(s_depth,3);
//uniform vec4 NumObjects;		// y = shadows enabled

bool outOfRange(float f){
    return f < 0 || f > 1;
}

layout(push_constant) uniform UniformBufferObject{
     mat4 viewProj;
    ivec4 viewRect;
} ubo;


void main()
{
	//calculate sampling positions using fragment pos and view dimensions
	vec2 texcoord = vec2(gl_FragCoord.x / ubo.viewRect[2], gl_FragCoord.y / ubo.viewRect[3]);
    
    // is this pixel visible to the light? if not, discard

    vec3 normal = texture(s_normal, texcoord).xyz;
    vec3 toLight = normalize(lightdir.xyz);

    int enabled = 1;
    const float bias = 0.005; //TODO: calcuate as a function of the difference between the normal and the light dir
#if 0
    if (NumObjects.y){
        vec4 sampledPos = texture(s_pos,texcoord);
        mat4 lightView = u_model[1];
        mat4 lightProj = u_model[0];
        mat4 vp = mul(lightProj,lightView);
        sampledPos = mul( vp,sampledPos);
        vec4 projected = sampledPos;
        sampledPos /= sampledPos.w; // perspective divide
        sampledPos.xy = sampledPos.xy * 0.5 + 0.5;    // transform to [0,1] 
        sampledPos.z *= -1;
        
        float sampledDepth = (outOfRange(sampledPos.x) || outOfRange(sampledPos.y)) ? 1 : texture(s_depth, sampledPos.xy).x;

        // in shadow
        if (sampledDepth.x < sampledPos.z - bias){
            enabled = false;
        }
    }
#endif
    
    float intensity = colorintensity[3];
    
    vec3 albedo = texture(s_albedo, texcoord).xyz;   
    
    float nDotL = max(dot(normal, toLight), 0);
    
    vec3 diffuseLight = albedo * nDotL;
    outcolor = vec4(intensity * colorintensity.xyz * diffuseLight * enabled, enabled);
	
}
