layout(location = 0) in vec2 a_position;

layout(push_constant, std430) uniform UniformBufferObject{
    mat4 viewProj;
    ivec4 viewRect; // for the whole screen
    ivec4 viewRegion; // for the virtual screen
    float radius;
    float bias;
    float power;
} ubo;

layout(location = 0) out flat vec4[4] outInvViewProj;

void main()
{
    mat4 invViewProj = inverse(ubo.viewProj);

    outInvViewProj = vec4[4](invViewProj[0], invViewProj[1], invViewProj[2], invViewProj[3]);

    
    gl_Position = vec4(a_position,1,1);
}
