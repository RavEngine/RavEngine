#extension GL_EXT_shader_explicit_arithmetic_types : enable

layout(location = 0) in vec2 a_position;
// per-instance
layout(location = 1) in vec4 inColorIntensity;
layout(location = 2) in vec3 inLightDir;
layout(location = 3) in uint inCastsShadows;    


layout(location = 0) out vec3 outLightDir;
layout(location = 1) out vec4 outColorIntensity;
layout(location = 2) out flat vec4[4] outInvViewProj; 

layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
    ivec4 viewRect;
    ivec4 viewRegion;   // for the virtual screen
    uint isRenderingShadows;
} ubo;

// https://stackoverflow.com/questions/37532640/making-a-nan-on-purpose-in-webgl
float makeNaN(float nonneg) {
    return sqrt(-nonneg-1.0);
}

void main()
{
    outColorIntensity = inColorIntensity;
    outLightDir = inLightDir;

    float NaN = makeNaN(1.0);

    mat4 invViewProj = inverse(ubo.viewProj);

    outInvViewProj = vec4[4](invViewProj[0],invViewProj[1],invViewProj[2],invViewProj[3]);

    gl_Position = (inCastsShadows == ubo.isRenderingShadows) ? vec4(a_position, 1, 1.0) : vec4(NaN,NaN,NaN,NaN);	//depth = 1
}
