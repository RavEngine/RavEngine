
struct UnlitOut{
    vec4 color;
};

#include "%s"

#include "mesh_shared.glsl"

void main(){
  UnlitOut user_out = frag();
  outcolor = user_out.color;

  #if RVE_TRANSPARENT
  writeTransparency(outcolor);
  #endif
}
