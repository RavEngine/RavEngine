
layout(push_constant, std430) uniform UniformBufferObject{
    ivec2 spritesheetDim;
    ivec2 numSprites;
} ubo;

struct ParticleData{
    vec3 pos;
    vec2 scale;
    uint animationFrame;
};

layout(location = 0) out vec2 out_uv; 


ParticleVertexOut vert(ParticleData data, ParticleMatrices matrices, vec2 inVertex){

    ParticleVertexOut v_out;

    const vec3 centerInViewSpace = (matrices.view * vec4(data.pos,1)).xyz;

    const vec3 offsetPoint = vec3(inVertex * data.scale, 0);

    vec4 vert = vec4(centerInViewSpace + offsetPoint,1);

    vert = matrices.proj * vert;

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

    v_out.position = vert;
    out_uv = uv;

    return v_out;
}
