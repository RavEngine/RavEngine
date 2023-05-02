
layout(location = 0) in vec2 a_position;

layout(push_constant) uniform UniformBufferObject{
	ivec4 viewRect;
} ubo;

void main()
{
    gl_Position = vec4(a_position,1,1);
}
