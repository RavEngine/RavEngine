
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec4 inColorIntensity;    // per-instance

layout(location = 0) out vec4 outColorIntensity;

layout(push_constant) uniform UniformBufferObject{
    ivec4 viewRect;
    uint ssao_enabled;
} ubo;

void main()
{
    gl_Position = vec4(a_position, 0, 1);
    outColorIntensity = inColorIntensity;
}