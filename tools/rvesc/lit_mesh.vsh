
#define MODEL_MATRIX_BINDING 10
#define ENTITY_INPUT_LOCATION 10

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inUV;
layout(location = ENTITY_INPUT_LOCATION) in uint inEntityID;
layout(std430, binding = MODEL_MATRIX_BINDING) readonly buffer modelMatrixBuffer{mat4 model[];};

struct LitVertexOut{
    vec4 position;
};

#include "%s"

void main(){
    mat4 inModel = model[inEntityID];

    LitVertexOut user_out = vertex(inModel);

    gl_Position = user_out.position;
}
