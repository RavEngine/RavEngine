

layout(location = 0) in vec2 inPosition;

struct EngineData{
    mat4 invView;
};

layout(scalar, binding = 0) readonly buffer engineDataSSBO
{
    EngineData constants;
};

void main()
{

    // render behind everything
    vec4 position = vec4(inPosition,0,1);
    
    gl_Position = position;
}
