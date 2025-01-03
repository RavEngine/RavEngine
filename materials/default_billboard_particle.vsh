
#include "RavEngine/billboard.glsl"

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

    BillboardVertexResult vertdata = billboard_vert(particleID, matrices, inVertex,
        ubo.spritesheetDim, ubo.numSprites,
        ubo.bytesPerParticle, ubo.particlePositionOffset, ubo.particleScaleOffset, ubo.particleFrameOffset
    );

    ParticleVertexOut v_out;

    v_out.localPosition = vertdata.localPosition;
    out_uv = vertdata.uv;
   
    return v_out;
}
