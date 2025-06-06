struct EngineData_Internal{
    mat4 viewProj;
    mat4 viewOnly;
    mat4 invView;
    mat4 projOnly;
    uvec4 screenDimensions;
    vec3 camPos;
    uvec3 gridSize;
    uvec2 outputDim;
    uint ambientLightCount;
    uint directionalLightCount;
    float zNear;
    float zFar;
    uint aoTextureBindlessID;
};
#define MODEL_MATRIX_BINDING 10

layout(std430, binding = MODEL_MATRIX_BINDING) readonly buffer modelMatrixBuffer{mat4 model[];};

layout(scalar, binding = 11) readonly buffer lightAuxDataSSBO{
    EngineData_Internal engineConstants[];
};
