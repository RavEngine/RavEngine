
layout(push_constant, std430) uniform UniformBufferObject{
    mat4 viewProj;
    ivec2 spritesheetDim;
    ivec2 numSprites;
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

layout(location = 0) in vec2 in_position;

layout(location = 0) out vec2 out_uv; 

void main(){

    uint particle = aliveParticleIndexBuffer[gl_InstanceID];
    ParticleData data = particleData[particle];

    vec4 vert = vec4(vec3(in_position * data.scale,0) + data.pos,1);

    vert = ubo.viewProj * vert;

    vec2 cellDim = (ubo.spritesheetDim / vec2(ubo.numSprites)) / ubo.spritesheetDim;  // [0,1] for a single frame

    uint frame = data.animationFrame;

    // using     
    vec2 uvs[] = {
        vec2(0,0),
        vec2(0,1) * cellDim,
        vec2(1,0) * cellDim,
        vec2(1,1) * cellDim
    };
    vec2 uv = uvs[gl_VertexID];

    float row = frame / ubo.numSprites.x;
    float col = frame % ubo.numSprites.y;

    row /= ubo.numSprites.y;
    col /= ubo.numSprites.x;

    uv += (vec2(col,row));

    gl_Position = vert;
    out_uv = uv;
}