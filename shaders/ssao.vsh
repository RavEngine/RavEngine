layout(location = 0) in vec2 a_position;

layout(push_constant, std430) uniform UniformBufferObject{
    mat4 viewProj;
    ivec4 viewRect; // for the whole screen
    ivec4 viewRegion; // for the virtual screen
} ubo;

void main()
{
    gl_Position = vec4(a_position,1,1);
}
