layout(location = 0) in vec2 a_position;

layout(push_constant, std430) uniform UniformBufferObject{
    mat4 viewProj;
    ivec4 viewRect; // for the whole screen
    ivec4 viewRegion; // for the virtual screen
} ubo;

layout(location = 0) out flat vec4 invViewProj[4];

void main()
{
    mat4 inv = inverse(ubo.viewProj);
    invViewProj[0] = inv[0];
    invViewProj[1] = inv[1];
    invViewProj[2] = inv[2];
    invViewProj[3] = inv[3];
     
    gl_Position = vec4(a_position,1,1);
}
