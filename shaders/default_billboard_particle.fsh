
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

layout(location = 0) in vec2 out_uv; 
layout(location = 1) flat in uint out_animationFrame;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 normal;

void main(){
    color = vec4(1,0,0,1);

    normal = vec4(0,0,1,1);
}