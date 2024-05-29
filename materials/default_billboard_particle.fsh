
layout(push_constant, std430) uniform UniformBufferObject{
    mat4 viewProj;
    ivec2 spritesheetDim;
    ivec2 spritesheetFrameDim;
} ubo;

layout(binding = 2) uniform sampler g_sampler; 
layout(binding = 3) uniform texture2D t_spritesheet;

layout(location = 0) in vec2 inUV; 

LitParticleOutput fragment(){
    LitParticleOutput fs_out;

    fs_out.color = texture(sampler2D(t_spritesheet, g_sampler), inUV);

    if (fs_out.color.a < 0.5){
        discard;
    }

    fs_out.normal = vec3(0,0,1);

    return fs_out;
}