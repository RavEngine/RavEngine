$input v_texcoord0

#include "common.sh"
SAMPLER2D(s_albedo,0);

void main()
{
    gl_FragColor = toLinear(texture2D(s_albedo, v_texcoord0));
}
