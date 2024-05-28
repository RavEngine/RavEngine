layout(location = 0) out vec4 outcolor;

layout(early_fragment_tests) in;

struct SkyboxFragmentOut{
    vec4 color;
};

#include "%s"

void main(){

    SkyboxFragmentOut user_out = fragment();

    outcolor = user_out.color;
}