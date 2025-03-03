
#define PRIMARY y
#define SECONDARY x

ivec2 sampleCoord(ivec2 pixel, int primaryCoord)
{
    return ivec2(pixel.x, primaryCoord);
}
#include "blur_kernel.glsl"