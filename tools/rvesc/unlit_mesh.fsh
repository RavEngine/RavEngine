
struct UnlitOut{
    vec4 color;
};

#include "%s"

layout(location = 0) out vec4 outcolor;

void main(){
  UnlitOut user_out = frag();
  outcolor = user_out.color;
}
