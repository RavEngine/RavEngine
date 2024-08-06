#define MODEL_MATRIX_BINDING 10
#define ENTITY_INPUT_LOCATION 10

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inUV;
layout(location = ENTITY_INPUT_LOCATION) in uint inEntityID;
layout(std430, binding = MODEL_MATRIX_BINDING) readonly buffer modelMatrixBuffer{mat4 model[];};

#include "lit_mesh_shared.glsl"

struct UnlitVertexOut{
    vec4 position;
};

struct EntityIn
{
    mat4 modelMtx;
    uint entityID;
};

#include "enginedata.glsl"

#include "%s"

#include "make_engine_data.glsl"

void main(){
    EntityIn entity;
    entity.entityID = inEntityID;
    entity.modelMtx = model[inEntityID];

    EngineData data = make_engine_data(engineConstants[0]);

    UnlitVertexOut user_out = vert(entity,data);

    gl_Position = user_out.position;
}
