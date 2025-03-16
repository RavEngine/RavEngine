
layout(binding = 0, r16f) uniform image2D imageIn;
layout(binding = 1, r16f) uniform image2D imageOut;

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

void main(){

    const ivec2 size = imageSize(imageIn);
    const ivec2 pixel = ivec2(gl_GlobalInvocationID.PRIMARY,gl_GlobalInvocationID.SECONDARY);
    // if outside the image, bail
    if (pixel.PRIMARY >= size.PRIMARY || pixel.SECONDARY >= size.SECONDARY)
    {
        return;
    }
    
    int dim = size.PRIMARY;
    
    uint nsampled = 1;
    int blurSize = 4;
    float current = imageLoad(imageIn, pixel).r;
    for (int i = pixel.PRIMARY - blurSize; i <= pixel.PRIMARY + blurSize; i++)
    {
        if (i < 0 || i > size.PRIMARY)
        {
            continue;
        }
        current += imageLoad(imageIn, sampleCoord(pixel, i)).r;
        nsampled++;
    }
    float result = current / nsampled;
    
    imageStore(imageOut, pixel, vec4(result));

}
