
layout(binding = 0, r16f) uniform image2D imageIn;
layout(binding = 1, r16f) uniform image2D imageOut;

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

void main(){

    const ivec2 size = imageSize(imageIn);
    const ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    if (pixel.x >= size.x || pixel.y >= size.y)
    {
        return;
    }
    
    int dim = size.x;
    
    uint nsampled = 1;
    int blurSize = 2;
    float current = imageLoad(imageIn, pixel).r;
    for (int i = pixel.x - blurSize; i <= pixel.x + blurSize; i++)
    {
        if (i < 0 || i > size.x)
        {
            continue;
        }
        current += imageLoad(imageIn, ivec2(i, pixel.y)).r;
        nsampled++;
    }
    float result = current / nsampled;
    
    imageStore(imageOut, pixel, vec4(result));

}
