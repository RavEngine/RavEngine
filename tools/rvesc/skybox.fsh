layout(location = 0) in vec3 v_sky_ray;
layout(location = 0) out vec4 outcolor;

layout(early_fragment_tests) in;

struct SkyboxFragmentOut{
    vec4 color;
};

struct SkyboxInput{
    vec3 skyRay;
};

#include "%s"

void main(){
    
    SkyboxFragmentOut user_out = frag(SkyboxInput(v_sky_ray));

    outcolor = user_out.color;
}
