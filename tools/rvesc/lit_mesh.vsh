#define ENTITY_INPUT_LOCATION 10

#include "lit_mesh_bindings.glsl"

struct LitVertexOut{
    vec3 localPosition;
};

struct EntityIn{
    mat4 modelMtx;
    uint entityID;
};

#include "enginedata.glsl"

#include "%s"

layout(location = ENTITY_INPUT_LOCATION) in uint inEntityID;

#define VARYINGDIR out
#include "mesh_varyings.glsl"


#include "lit_mesh_shared.glsl"
#include "make_engine_data.glsl"

void main(){
    EntityIn entity;
    entity.entityID = inEntityID;
    entity.modelMtx = model[inEntityID];

    EngineData data = make_engine_data(engineConstants[0]);

    LitVertexOut user_out = vert(entity, data);
    
    vec4 worldPos = entity.modelMtx * vec4(user_out.localPosition, 1);

    gl_Position = data.viewProj * worldPos;
    worldPosition = worldPos.xyz;
    clipSpaceZ = gl_Position.z;
    viewPosition = (engineConstants[0].viewOnly * vec4(worldPosition,1)).xyz;
    varyingEntityID = inEntityID;
}
