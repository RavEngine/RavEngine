
layout(push_constant, std430) uniform UniformBufferObject{
    ivec2 spritesheetDim;
    ivec2 numSprites;
    uint bytesPerParticle;
    uint particlePositionOffset;
    uint particleScaleOffset;
    uint particleFrameOffset;
} ubo;

layout(location = 0) out vec2 out_uv; 

ParticleVertexOut vert(uint particleID, ParticleMatrices matrices, vec2 inVertex){

    // load the data
    const uint particleDataOffset = particleID * ubo.bytesPerParticle;

    const uint posOffset = particleDataOffset + ubo.particlePositionOffset / 4;
    vec3 data_pos = vec3(
        uintBitsToFloat(particleDataBytes[posOffset]),
        uintBitsToFloat(particleDataBytes[posOffset+1]),
        uintBitsToFloat(particleDataBytes[posOffset+2])
    );

    const uint scaleOffset = particleDataOffset + ubo.particleScaleOffset / 4;

    vec2 data_scale = vec2(
        uintBitsToFloat(particleDataBytes[scaleOffset]),
        uintBitsToFloat(particleDataBytes[scaleOffset+1])
    );

    ParticleVertexOut v_out;

    mat3 billboardmtx = matrices.billboard;

    vec3 rotated_quad = billboardmtx * vec3(inVertex * data_scale,0);

    vec4 vert = vec4(data_pos + rotated_quad,1);

    vert = matrices.viewProj * vert;

    vec2 cellDim = (ubo.spritesheetDim / vec2(ubo.numSprites)) / ubo.spritesheetDim;  // [0,1] for a single frame

    uint frame = particleDataBytes[particleDataOffset + ubo.particleFrameOffset / 4];

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
