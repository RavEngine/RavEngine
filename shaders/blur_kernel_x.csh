#define PRIMARY x
#define SECONDARY y

ivec2 sampleCoord(ivec2 pixel, int primaryCoord)
{
    return ivec2(primaryCoord, pixel.y);
}
#include "blur_kernel.glsl"