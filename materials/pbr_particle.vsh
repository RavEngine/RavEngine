layout(push_constant, std430) uniform UniformBufferObject{
    uint bytesPerParticle;
    uint positionOffset;
} ubo;

ParticleVertexOut vert(uint particleID, ParticleMatrices matrices){
    const uint particleDataOffset = (particleID * ubo.bytesPerParticle) / 4;
    const uint posOffset = particleDataOffset + ubo.positionOffset / 4;

    vec3 data_pos = vec3(
        uintBitsToFloat(particleDataBytes[posOffset]),
        uintBitsToFloat(particleDataBytes[posOffset+1]),
        uintBitsToFloat(particleDataBytes[posOffset+2])
    );

    ParticleVertexOut vs_out;


    vec4 finalPos = matrices.viewProj * vec4(inPosition + data_pos,1);

    vs_out.position = finalPos;

    return vs_out;

}