
#define RVE_AO 1
#include "ssgi_ao.glsl"

void main() {
    fragColor = getVisibility();
}