
#define MODEL_MATRIX_BINDING 10
#define ENTITY_INPUT_LOCATION 10

#include "lit_mesh_bindings.glsl"

layout(location = ENTITY_INPUT_LOCATION) in uint inEntityID;

layout(location = 11) out vec3 worldPosition;

layout(std430, binding = MODEL_MATRIX_BINDING) readonly buffer modelMatrixBuffer{mat4 model[];};

struct LitVertexOut{
    vec4 position;
    vec3 worldPosition;
};

struct EntityIn{
    mat4 modelMtx;
    uint entityID;
};

#include "%s"

void main(){
    EntityIn entity;
    entity.entityID = inEntityID;
    entity.modelMtx = model[inEntityID];

    LitVertexOut user_out = vert(entity);

    gl_Position = user_out.position;
    worldPosition = user_out.worldPosition;
}
