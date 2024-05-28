
struct UnlitOut{
    vec4 color;
};

#include "%s"

layout(location = 0) out vec4 outcolor;

void main(){
  UnlitOut user_out = fragment();
  outcolor = user_out.color;
}