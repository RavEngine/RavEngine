
layout(push_constant, std430) uniform UniformBufferObject{
    mat4 viewProj;
    ivec2 spritesheetDim;
    ivec2 spritesheetFrameDim;
} ubo;

struct ParticleData{
    vec3 pos;
    vec2 scale;
    uint animationFrame;
};

layout(scalar, binding = 0) readonly buffer particleDataSSBO
{
    ParticleData particleData[];
};

layout(std430, binding = 1) readonly buffer aliveSSBO
{
    uint aliveParticleIndexBuffer[];
};
layout(binding = 2) uniform sampler g_sampler; 
layout(binding = 3) uniform texture2D t_spritesheet;

layout(location = 0) in vec2 inUV; 

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 normal;

void main(){
    color = texture(sampler2D(t_spritesheet, g_sampler), inUV);

    if (color.a < 0.5){
        discard;
    }

    normal = vec4(0,0,1,1);
}