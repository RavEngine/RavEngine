
// adapted from:https://github.com/HanetakaChou/MultiLayerAlphaBlending/

layout(binding = 0, rgba16f) uniform image2D mlabAccum0;
layout(binding = 1, rgba8) uniform image2D mlabAccum1;
layout(binding = 2, rgba8) uniform image2D mlabAccum2;
layout(binding = 3, rgba8) uniform image2D mlabAccum3;

layout(location = 0) out vec4 outcolor;

struct OITData_4Layer_NoDepth
{
    vec4 ACV[4];
};

void main()
{
    const ivec2 pixelCoord = ivec2( gl_FragCoord.xy);
    OITData_4Layer_NoDepth oitData;
    oitData.ACV[0] = imageLoad(mlabAccum0, pixelCoord);
    oitData.ACV[1] = imageLoad(mlabAccum1, pixelCoord);
    oitData.ACV[2] = imageLoad(mlabAccum2, pixelCoord);
    oitData.ACV[3] = imageLoad(mlabAccum3, pixelCoord);
    
    //Under Operation
    const int numLayers = 4;
    vec3 CFinal = vec3(0.0f,0.0f,0.0f);
    float AlphaTotal = 1.0f;
    for (int i = 0; i < numLayers; ++i)
    {
        CFinal += oitData.ACV[i].rgb * AlphaTotal;
        AlphaTotal *= oitData.ACV[i].a;
    }

    // why 1-Alpha? because pixels that haven't been written to have (0,0,0,1) as their values.
    // while Alpha contains the real alpha. We need to use OneMinusSrcAlpha when blending.
    // without inversion, this would result in all unwritten pixels to get set to black.
    // Therefore, we invert it here, and OneMinusSrcAlpha re-inverts it, producing the correct result.
    outcolor = vec4(CFinal, 1-AlphaTotal);
}
