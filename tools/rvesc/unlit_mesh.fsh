
struct UnlitOut{
    vec4 color;
};

#include "%s"

#include "mesh_shared.glsl"

void main(){
  UnlitOut user_out = frag();
  vec4 outcolor = user_out.color;

  #if RVE_TRANSPARENT
    writeTransparency(outcolor);
 #else
    result = outcolor;
  #endif
}
