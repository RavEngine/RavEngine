
struct LitParticleOutput{
    vec4 color;
    vec3 normal;
};

#include "%s"

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 normal;

void main(){

    LitParticleOutput user_out = fragment();

    color = user_out.color;
    normal = vec4(user_out.normal,1);
}