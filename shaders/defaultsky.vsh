

layout(location = 0) in vec2 inPosition;

layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
    //mat4 model;
    //float timeSinceStart;
} ubo;

void main()
{
    // we are only interested in the non-translation part of the matrix
    mat3 rotScaleOnly = mat3(ubo.viewProj);
    
    // render behind everything
    vec4 position = vec4(inPosition,0,1);
    
    gl_Position = position;
}
