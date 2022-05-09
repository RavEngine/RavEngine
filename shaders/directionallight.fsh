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
BUFFER_RO(all_vb, float, 12);
BUFFER_RO(all_ib, uint, 13);

bool outOfRange(float f){
    return f < 0 || f > 1;
}

float sign (vec2 p1, vec2 p2, vec2 p3)
{
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool PointInTriangle (vec2 pt, vec2 v1, vec2 v2, vec2 v3)
{
    float d1, d2, d3;
    bool has_neg, has_pos;

    d1 = sign(pt, v1, v2);
    d2 = sign(pt, v2, v3);
    d3 = sign(pt, v3, v1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

EARLY_DEPTH_STENCIL
void main()
{
	//calculate sampling positions using fragment pos and view dimensions
	vec2 texcoord = vec2(gl_FragCoord.x / u_viewRect[2], gl_FragCoord.y / u_viewRect[3]);
    
    // is this pixel visible to the light? if not, discard

    vec3 normal = texture2D(s_normal, texcoord);
    vec3 toLight = normalize(lightdir.xyz);

    bool enabled = 1;
    const float bias = 0.005; //TODO: calcuate as a function of the difference between the normal and the light dir
    if (NumObjects.y){
        vec4 sampledPos = texture2D(s_pos,texcoord);
        mat4 lightView = u_model[1];
        mat4 lightProj = u_model[0];
        mat4 vp = mul(lightProj,lightView);
        sampledPos = mul( vp,sampledPos);
        vec4 projected = sampledPos;
        sampledPos /= sampledPos.w; // perspective divide
        sampledPos.xy = sampledPos.xy * 0.5 + 0.5;    // transform to [0,1] 
        sampledPos.z *= -1;

        uint count = 0;
        for(int r = -1; r <= 1; r++){
            for(int c = -1; c <= 1; c++){
                vec2 offset = vec2((1.0 / u_viewRect[2]) * r,(1.0 / u_viewRect[3]) * c);
                float sampledDepth = (outOfRange(sampledPos.x) || outOfRange(sampledPos.y)) ? 1 : texture2D(s_depth, sampledPos.xy + offset).x;
                if (sampledDepth.x < sampledPos.z - bias){
                    count++;
                }
            }
        }

        if (count >= 9){
            enabled = false;
        }
        else if (count == 0){
            enabled = true;
        }
        else{

            bool isInside = false;
            float sampledIdx = texture2D(s_depthdata, sampledPos.xy).x * 3;    // 3 indices for each triangle
            uint p1i = all_ib[sampledIdx];
            uint p2i = all_ib[sampledIdx+1];
            uint p3i = all_ib[sampledIdx+2];
            vec3 p1 = vec3(all_vb[p1i*3],all_vb[p1i*3+1],all_vb[p1i*3+2]);
            vec3 p2 = vec3(all_vb[p2i*3],all_vb[p2i*3+1],all_vb[p2i*3+2]);
            vec3 p3 = vec3(all_vb[p3i*3],all_vb[p3i*3+1],all_vb[p3i*3+2]);

            p1 = mul(vp, p1);
            p2 = mul(vp, p2);
            p3 = mul(vp, p3);
            isInside = !PointInTriangle(projected.xy,p1.xy,p2.xy,p3.xy);
            
            float sampledDepth = (outOfRange(sampledPos.x) || outOfRange(sampledPos.y)) ? 1 : texture2D(s_depth, sampledPos.xy).x;
            if (sampledDepth.x < sampledPos.z - bias){
                enabled = isInside;
            }
        }
    }
    
    float intensity = colorintensity[3];
    
    vec3 albedo = texture2D(s_albedo, texcoord);
    
    
    float nDotL = max(dot(normal, toLight), 0);
    
    vec3 diffuseLight = albedo * nDotL;
    gl_FragData[0] = vec4(intensity * colorintensity.xyz * diffuseLight * enabled, enabled);
	
}
