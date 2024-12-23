
struct UnlitOut{
    vec4 color;
};

#include "%s"

#include "mesh_shared.glsl"

void main(){
  UnlitOut user_out = frag();
  vec4 outcolor = user_out.color;

  #if !RVE_DEPTHONLY 
      #if RVE_TRANSPARENT
        beginInvocationInterlockARB();
        writeTransparency(outcolor);
        endInvocationInterlockARB();
     #else
        result = outcolor;
      #endif
  #endif
}
