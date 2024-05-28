struct LitOutput{
    vec4 color;
    vec3 normal;
    float roughness;
    float specular;
    float metallic;
    float ao;
};

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec3[3] inTBN;

#include "%s"

layout(location = 0) out vec4 outcolor;
layout(location = 1) out vec4 outnormal;
layout(location = 2) out vec4 outRoughnessSpecularMetalicAO;

void main(){

    LitOutput user_out = fragment();

    outcolor = user_out.color;
    outnormal = vec4(user_out.normal,1);
    outRoughnessSpecularMetalicAO = vec4(user_out.roughness, user_out.specular, user_out.metallic, user_out.ao);
}
