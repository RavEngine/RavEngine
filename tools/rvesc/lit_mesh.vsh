
#define MODEL_MATRIX_BINDING 10
#define ENTITY_INPUT_LOCATION 10

#include "lit_mesh_bindings.glsl"

struct LitVertexOut{
    vec4 position;
    vec3 worldPosition;
};

struct EntityIn{
    mat4 modelMtx;
    uint entityID;
};

#include "%s"

layout(location = ENTITY_INPUT_LOCATION) in uint inEntityID;

layout(location = 11) out vec3 worldPosition;
layout(location = 12) out vec3 viewPosition;

layout(std430, binding = MODEL_MATRIX_BINDING) readonly buffer modelMatrixBuffer{mat4 model[];};

#include "lit_mesh_shared.glsl"

void main(){
    EntityIn entity;
    entity.entityID = inEntityID;
    entity.modelMtx = model[inEntityID];

    LitVertexOut user_out = vert(entity);

    gl_Position = user_out.position;
    worldPosition = user_out.worldPosition;
    viewPosition = (engineConstants[0].viewOnly * vec4(worldPosition,1)).xyz;
}
