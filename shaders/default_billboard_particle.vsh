
layout(push_constant, std430) uniform UniformBufferObject{
    mat4 viewProj;
    vec2 spritesheetDim;
    vec2 spritesheetFrameDim;

} ubo;

struct ParticleData{
    vec3 pos;
    vec2 scale;
    uint animationFrame;
    float age;
};

layout(std430, binding = 0) readonly buffer particleDataSSBO
{
    ParticleData particleData[];
};

layout(std430, binding = 1) readonly buffer aliveSSBO
{
    uint aliveParticleIndexBuffer[];
};

layout(location = 0) in vec2 in_position;

layout(location = 0) out vec2 out_uv; 
layout(location = 1) out uint out_animationFrame;

void main(){

    uint particle = aliveParticleIndexBuffer[gl_InstanceID];
    ParticleData data = particleData[particle];

    vec4 vert = vec4(vec3(in_position,0) + data.pos,1);
    vert = ubo.viewProj * vert;

    gl_Position = vert;
    out_uv = vec2(0,0);
    out_animationFrame = data.animationFrame;
}