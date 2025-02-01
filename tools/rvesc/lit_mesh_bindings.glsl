
layout(location = 0) in vec3 inPosition;
#if !RVE_DEPTHONLY
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
#endif
layout(location = 4) in vec2 inUV;