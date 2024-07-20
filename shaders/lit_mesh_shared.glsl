struct EngineData{
    mat4 viewProj;
    mat4 viewOnly;
    mat4 projOnly;
    vec3 camPos;
    uvec3 gridSize;
    uvec2 screenDimensions;
    uint ambientLightCount;
    uint directionalLightCount;
    float zNear;
    float zFar;
};

layout(scalar, binding = 11) readonly buffer lightAuxDataSSBO{
    EngineData engineConstants[];
};
