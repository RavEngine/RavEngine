#define MODEL_MATRIX_BINDING 10
#define ENTITY_INPUT_LOCATION 10

#include "lit_mesh_bindings.glsl"

#include "lit_mesh_shared.glsl"

struct UnlitVertexOut{
    vec3 localPosition;
};

struct EntityIn
{
    mat4 modelMtx;
    uint entityID;
};

#include "enginedata.glsl"

#include "%s"

layout(location = ENTITY_INPUT_LOCATION) in uint inEntityID;

#define VARYINGDIR out
#include "mesh_varyings.glsl"

#include "make_engine_data.glsl"

void main(){
    EntityIn entity;
    entity.entityID = inEntityID;
    entity.modelMtx = model[inEntityID];

    EngineData data = make_engine_data(engineConstants[0]);

    UnlitVertexOut user_out = vert(entity,data);
    
    vec4 worldPos = entity.modelMtx * vec4(user_out.localPosition, 1);

    inTBN[0] = inTangent;
	inTBN[1] = inBitangent;
	inTBN[2] = inNormal;

    gl_Position = data.viewProj * worldPos;
    clipSpaceZ = gl_Position.z;
    varyingEntityID = inEntityID;
}
