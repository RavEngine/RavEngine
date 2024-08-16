
struct UnlitOut{
    vec4 color;
};

#include "%s"

layout(location = 13) in float clipSpaceZ;

layout(location = 0) out vec4 outcolor;
#if RVE_TRANSPARENT
layout(location = 1) out float revealage;
#endif

#include "mesh_shared.glsl"

void main(){
  UnlitOut user_out = frag();
  outcolor = user_out.color;

  #if RVE_TRANSPARENT
  writeTransparency(outcolor);
  #endif
}
