#define VARYINGDIR in
#include "mesh_varyings.glsl"

#if RVE_TRANSPARENT
layout(binding = 0, rgba16f) uniform image2D mlabAccum0;
layout(binding = 1, rgba8) uniform image2D mlabAccum1;
layout(binding = 2, rgba8) uniform image2D mlabAccum2;
layout(binding = 3, rgba8) uniform image2D mlabAccum3;
layout(binding = 4, rgba16f) uniform image2D mlabDepth;
#else
    layout(location = 0) out vec4 result;
#endif

void writeTransparency(inout vec4 outcolor){
    #if RVE_TRANSPARENT
    
    #endif
}
