
struct BillboardVertexResult{
    vec3 localPosition;
    vec2 uv;
};

BillboardVertexResult billboard_vert(uint particleID, ParticleMatrices matrices, vec2 inVertex, ivec2 spritesheetDim, ivec2 numSprites, uint bytesPerParticle, uint particlePositionOffset, uint particleScaleOffset, uint particleFrameOffset){

    // load the data
    const uint particleDataOffset = (particleID * bytesPerParticle) / 4;

    const uint posOffset = particleDataOffset + particlePositionOffset / 4;
    vec3 data_pos = vec3(
        uintBitsToFloat(particleDataBytes[posOffset]),
        uintBitsToFloat(particleDataBytes[posOffset+1]),
        uintBitsToFloat(particleDataBytes[posOffset+2])
    );

    const uint scaleOffset = particleDataOffset + particleScaleOffset / 4;

    vec2 data_scale = vec2(
        uintBitsToFloat(particleDataBytes[scaleOffset]),
        uintBitsToFloat(particleDataBytes[scaleOffset+1])
    );

    BillboardVertexResult v_out;

    mat3 billboardmtx = matrices.billboard;

    vec3 rotated_quad = billboardmtx * vec3(inVertex * data_scale,0);

    vec3 vert = data_pos + rotated_quad;

    vec2 cellDim = (spritesheetDim / vec2(numSprites)) / spritesheetDim;  // [0,1] for a single frame

    uint frame = particleDataBytes[particleDataOffset + particleFrameOffset / 4];

    // using     
    vec2 uvs[] = {
        vec2(0,0),
        vec2(0,1) * cellDim,
        vec2(1,0) * cellDim,
        vec2(1,1) * cellDim
    };
    vec2 uv = uvs[gl_VertexIndex];

    float row = frame / numSprites.x;
    float col = frame % numSprites.y;

    row /= numSprites.y;
    col /= numSprites.x;

    uv += (vec2(col,row));

    v_out.localPosition = vert;
    v_out.uv = uv;

    return v_out;
}
