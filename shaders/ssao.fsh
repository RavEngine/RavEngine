
#include "ravengine_shader.glsl"

layout(binding = 0) uniform sampler srcSampler;
layout(binding = 1) uniform texture2D normalTex;
layout(binding = 2) uniform texture2D depthTex;

layout(location = 0) out vec4 outcolor;

void main(){
    outcolor = vec4(1,1,1,1);
}
