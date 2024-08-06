
layout(push_constant, std430) uniform UniformBufferObject{
	vec4 colorTint;
	float metallicTint;
	float roughnessTint;
	float specularTint;
    uint bytesPerParticle;
    uint positionOffset;
    uint scaleOffset;
    uint rotationOffset;
} ubo;

#include "RavEngine/quat.glsl"

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec3[3] outTBN;

ParticleVertexOut vert(uint particleID, ParticleMatrices matrices){
    const uint particleDataOffset = (particleID * ubo.bytesPerParticle) / 4;
    const uint posOffset = particleDataOffset + ubo.positionOffset / 4;

    vec3 data_pos = vec3(
        uintBitsToFloat(particleDataBytes[posOffset]),
        uintBitsToFloat(particleDataBytes[posOffset+1]),
        uintBitsToFloat(particleDataBytes[posOffset+2])
    );

    const uint scaleOffset = particleDataOffset + ubo.scaleOffset / 4;
    vec3 scale_pos = vec3(
        uintBitsToFloat(particleDataBytes[scaleOffset]),
        uintBitsToFloat(particleDataBytes[scaleOffset+1]),
        uintBitsToFloat(particleDataBytes[scaleOffset+2])
    );

    const uint rotationOffset = particleDataOffset + ubo.rotationOffset / 4;
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

	outTBN[0] = T;
	outTBN[1] = B;
	outTBN[2] = N;

    outUV = inUV;

    ParticleVertexOut vs_out;

    vec4 vert = inModel * vec4(inPosition, 1);

    vec4 finalPos = matrices.viewProj * vert;

    vs_out.position = finalPos;
    vs_out.worldPosition = data_pos;

    return vs_out;

}