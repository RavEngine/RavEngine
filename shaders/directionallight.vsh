
layout(location = 0) in vec2 a_position;
// per-instance
layout(location = 1) in vec4 inColorIntensity;
layout(location = 2) in vec3 inLightDir;       


layout(location = 0) out vec3 outLightDir;
layout(location = 1) out vec4 outColorIntensity;


layout(push_constant) uniform UniformBufferObject{
    ivec4 viewRect;
} ubo;

void main()
{
    outColorIntensity = inColorIntensity;
    outLightDir = inLightDir;

    gl_Position = vec4(a_position, 1, 1.0);	//depth = 1
}
