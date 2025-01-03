
#include "RavEngine/quat.glsl"

struct MeshParticleVertexOut{
    vec3 localPosition;
    vec3 T;
    vec3 B;
    vec3 N;
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

    vec3 T = normalize(vec3(inModel * vec4(inTangent,   0.0)));
   	vec3 B = normalize(vec3(inModel * vec4(inBitangent, 0.0)));
   	vec3 N = normalize(vec3(inModel * vec4(inNormal,    0.0)));

    MeshParticleVertexOut vs_out;

	vs_out.T = T;
	vs_out.B = B;
	vs_out.N = N;

    vec4 vert = inModel * vec4(inPosition, 1);

    vs_out.localPosition = data_pos;

    return vs_out;

}