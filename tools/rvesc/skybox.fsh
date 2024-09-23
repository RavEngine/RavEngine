layout(location = 0) in vec3 v_sky_ray;
layout(location = 0) out vec4 outcolor;

layout(early_fragment_tests) in;

struct SkyboxFragmentOut{
    vec4 color;
};

struct SkyboxInput{
    vec3 skyRay;
};

struct EngineData{
    mat3 invView;
    vec3 camPos;
    float fov;
    float aspectRatio;
};

layout(scalar, binding = 1) readonly buffer engineDataSSBO
{
    EngineData constants;
};

#include "%s"

void main(){
    
    SkyboxFragmentOut user_out = frag(SkyboxInput(v_sky_ray));

    outcolor = user_out.color;
}
