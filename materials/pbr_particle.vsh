
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

#include "RavEngine/meshparticle.glsl"

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec3[3] outTBN;

ParticleVertexOut vert(uint particleID, ParticleMatrices matrices){

    MeshParticleVertexOut mesh_out = mesh_particle_vert(particleID, matrices,
        ubo.bytesPerParticle, ubo.positionOffset, ubo.scaleOffset, ubo.rotationOffset
    );

    outUV = inUV;

    outTBN[0] = mesh_out.T;
    outTBN[1] = mesh_out.B;
    outTBN[2] = mesh_out.N;

    ParticleVertexOut vs_out;
    vs_out.localPosition = mesh_out.localPosition;

    return vs_out;
}