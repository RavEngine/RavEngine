
// adapted from https://vkguide.dev/docs/gpudriven/compute_culling/
layout(binding = 0, r32f) uniform writeonly image2D outImage;

layout(binding = 1) uniform texture2D inImage;
layout(binding = 2) uniform sampler g_sampler;

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
    ivec2 dim = imageSize(outImage);
    uvec2 pos = gl_GlobalInvocationID.xy;

    if (pos.x >= dim.x || pos.y >= dim.y){
        return;
    }
    
    
    // Sampler is set up to do min reduction, so this computes the minimum depth of a 2x2 texel quad
    float depth = texture(sampler2D(inImage, g_sampler), (vec2(pos) + vec2(0.5)) / dim).x;

    imageStore(outImage, ivec2(pos), vec4(depth));
}
