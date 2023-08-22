
#include "ravengine_shader_defs.h"

#define VS_INPUTS() \
layout(location = 0) in vec3 inPosition;\
layout(location = 1) in vec3 inNormal;\
layout(location = 2) in vec3 inTangent;\
layout(location = 3) in vec3 inBitangent;\
layout(location = 4) in vec2 inUV;\
layout(location = ENTITY_INPUT_LOCATION) in uint inEntityID;\
layout(std430, binding = MODEL_MATRIX_BINDING) readonly buffer modelMatrixBuffer{mat4 model[];};