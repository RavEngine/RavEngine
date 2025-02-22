
layout(push_constant, std430) uniform UniformBufferObject{
    ivec2 spritesheetDim;
    ivec2 spritesheetFrameDim;
} ubo;

layout(binding = 0) uniform sampler g_sampler; 
layout(binding = 1) uniform texture2D t_spritesheet;

layout(location = 0) in vec2 inUV; 

LitOutput frag(EnvironmentData envData){
    LitOutput fs_out;

    fs_out.color = texture(sampler2D(t_spritesheet, g_sampler), inUV);

    if (fs_out.color.a < 0.5){
        discard;
    }

    fs_out.normal = vec3(0,0,1);

    fs_out.roughness = 1;
    fs_out.specular = 0;
    fs_out.metallic = 0;
    fs_out.ao = 1;
    fs_out.emissiveColor = vec3(0,0,0);

    return fs_out;
}
