#include "common.sh"
#include <bgfx_compute.sh>

BUFFER_RW(lightdata, uint, 1);		// the amalgamated index buffer
uniform vec4 NumObjects;            // x = width, y = height

void main(){
    lightdata[gl_FragCoord.y * NumObjects.x + gl_FragCoord.x] = 0;
    gl_FragColor = vec4(0,0,0,0);
}
