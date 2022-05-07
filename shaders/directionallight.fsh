$input lightdir, colorintensity, lightID

#include "common.sh"
#include <bgfx_shader.sh>
#include <bgfx_compute.sh>
SAMPLER2D(s_albedo,0);
SAMPLER2D(s_normal,1);
SAMPLER2D(s_pos,2);
SAMPLER2D(s_depth,3);
SAMPLER2D(s_depthdata,4);
uniform vec4 NumObjects;		// y = shadows enabled

EARLY_DEPTH_STENCIL
void main()
{
	//calculate sampling positions using fragment pos and view dimensions
	vec2 texcoord = vec2(gl_FragCoord.x / u_viewRect[2], gl_FragCoord.y / u_viewRect[3]);
    
    // is this pixel visible to the light? if not, discard

    vec3 normal = texture2D(s_normal, texcoord);
    vec3 toLight = normalize(lightdir.xyz);

    bool enabled = 1;
    if (NumObjects.y){
        vec4 sampledPos = texture2D(s_pos,texcoord);
        mat4 lightView = u_model[1];
        mat4 lightProj = u_model[0];
        sampledPos = mul( mul(lightProj,lightView),sampledPos);
        sampledPos /= sampledPos.w; // perspective divide
        sampledPos.xy = sampledPos.xy * 0.5 + 0.5;    // transform to [0,1] 
        sampledPos.z *= -1;
        vec4 sampledDepth = texture2D(s_depth, sampledPos.xy);

        float bias = 0.005; //TODO: calcuate as a function of the difference between the normal and the light dir

        if (sampledDepth.x < sampledPos.z - bias){
            enabled = false;
        }
    }
    
    float intensity = colorintensity[3];
    
    vec3 albedo = texture2D(s_albedo, texcoord);
    
    
    float nDotL = max(dot(normal, toLight), 0);
    
    vec3 diffuseLight = albedo * nDotL;
    gl_FragData[0] = vec4(intensity * colorintensity.xyz * diffuseLight * enabled, enabled);
	
}
