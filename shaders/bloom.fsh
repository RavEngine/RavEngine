
layout(push_constant, std430) uniform UniformBufferObject{
    ivec2 targetDim;
} ubo;

layout(binding = 0) uniform texture2D colorFramebuffer;
layout(binding = 1) uniform sampler g_sampler;


layout(location = 0) out vec4 outcolor;

void main(){
    
    //TODO: optimize this (single-directional blur + shared memory?)
    const int radius = 5;
    vec2 radiusNorm = radius / vec2(ubo.targetDim);
    
    uint totalSampled = 0;
    vec3 colorSum = vec3(0,0,0);
    vec2 uv = gl_FragCoord.xy / ubo.targetDim;
    
    for(int x = -radius; x < radius; x++){
        for(int y = -radius; y < radius; y++){
            vec2 samplePos = vec2(x,y) * radiusNorm + uv;
            if (samplePos.x >= 0 && samplePos.y >= 0){
                vec3 color = texture(sampler2D(colorFramebuffer, g_sampler), samplePos).xyz;
                vec3 colorToAdd = max(vec3(0), color - 1);
                colorSum += colorToAdd;
                totalSampled ++;
            }
        }
    }
    
    colorSum /= totalSampled;
    
    vec4 origColor = texture(sampler2D(colorFramebuffer, g_sampler), uv);
        
    outcolor = origColor + vec4(colorSum, 0);
}
