
layout(push_constant, std430) uniform UniformBufferObject{
    ivec2 spritesheetDim;
    ivec2 spritesheetFrameDim;
} ubo;

layout(binding = 0) uniform sampler g_sampler; 
layout(binding = 1) uniform texture2D t_spritesheet;

layout(location = 0) in vec2 inUV; 

LitParticleOutput frag(){
    LitParticleOutput fs_out;

    fs_out.color = texture(sampler2D(t_spritesheet, g_sampler), inUV);

    if (fs_out.color.a < 0.5){
        discard;
    }

    fs_out.normal = vec3(0,0,1);

    return fs_out;
}
