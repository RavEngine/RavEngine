
#include "RavEngine/quat.glsl"

struct MeshParticleVertexOut{
    vec3 localPosition;
};

MeshParticleVertexOut mesh_particle_vert(uint particleID, ParticleMatrices matrices, uint bytesPerParticle, uint in_positionOffset, uint in_scaleOffset, uint in_rotationOffset){
    const uint particleDataOffset = (particleID * bytesPerParticle) / 4;
    const uint posOffset = particleDataOffset + in_positionOffset / 4;

    vec3 data_pos = vec3(
        uintBitsToFloat(particleDataBytes[posOffset]),
        uintBitsToFloat(particleDataBytes[posOffset+1]),
        uintBitsToFloat(particleDataBytes[posOffset+2])
    );

    const uint scaleOffset = particleDataOffset + in_scaleOffset / 4;
    vec3 scale_pos = vec3(
        uintBitsToFloat(particleDataBytes[scaleOffset]),
        uintBitsToFloat(particleDataBytes[scaleOffset+1]),
        uintBitsToFloat(particleDataBytes[scaleOffset+2])
    );

    const uint rotationOffset = particleDataOffset + in_rotationOffset / 4;
    vec4 rot_quat = vec4(
        uintBitsToFloat(particleDataBytes[rotationOffset]),
        uintBitsToFloat(particleDataBytes[rotationOffset+1]),
        uintBitsToFloat(particleDataBytes[rotationOffset+2]),
        uintBitsToFloat(particleDataBytes[rotationOffset+3])
    );

    mat3 rotMat_only = quatToMat3(rot_quat);
    mat4 rotMat = mat3toMat4(rotMat_only);

    mat4 translateMat = mat4(1);
    translateMat[3] = vec4(data_pos, 1);

    mat4 scaleMat = mat4(1);
    scaleMat[0][0] = scale_pos.x;
    scaleMat[1][1] = scale_pos.y;
    scaleMat[2][2] = scale_pos.z;

    mat4 inModel = translateMat * rotMat * scaleMat;

    MeshParticleVertexOut vs_out;

    vec4 vert = inModel * vec4(inPosition, 1);

    vs_out.localPosition = vert.xyz;

    return vs_out;

}
