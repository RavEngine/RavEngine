#define VARYINGDIR in
#include "mesh_varyings.glsl"

#if RVE_TRANSPARENT
layout(binding = 23, rgba16f) uniform image2D mlabAccum0;
layout(binding = 24, rgba8) uniform image2D mlabAccum1;
layout(binding = 25, rgba8) uniform image2D mlabAccum2;
layout(binding = 26, rgba8) uniform image2D mlabAccum3;
layout(binding = 27, rgba16f) uniform image2D mlabDepth;
#else
    layout(location = 0) out vec4 result;
#endif

// adapted from: https://github.com/HanetakaChou/MultiLayerAlphaBlending/
// note: in the original, these use half-floats
struct OITData_4Layer
{
    vec4 ACV[4];
    float D[4];
};

void writeTransparency(inout vec4 baseColorSample){
    #if RVE_TRANSPARENT
    vec4 newACV = vec4(baseColorSample.a*baseColorSample.rgb, 1 - baseColorSample.a);
    float newD = gl_FragCoord.z;
    
    //Read KBuffer
    const ivec2 pixelCoord = ivec2( gl_FragCoord.xy);
    OITData_4Layer oitData;
    oitData.ACV[0] = imageLoad(mlabAccum0, pixelCoord);
    oitData.ACV[1] = imageLoad(mlabAccum1, pixelCoord);
    oitData.ACV[2] = imageLoad(mlabAccum2, pixelCoord);
    oitData.ACV[3] = imageLoad(mlabAccum3, pixelCoord);
    
    const vec4 depthValues = imageLoad(mlabDepth, pixelCoord);
    oitData.D[0] = depthValues.r;
    oitData.D[1] = depthValues.g;
    oitData.D[2] = depthValues.b;
    oitData.D[3] = depthValues.a;
    
    //Modify KBuffer
    const int numLayers = 4;
    const int lastLayer = numLayers - 1;

    //Insert
    for (int i = 0; i < numLayers; ++i)
    {
        vec4 layerACV = oitData.ACV[i];
        float layerD = oitData.D[i];

        bool insert = (newD <= layerD);
        //Insert
        oitData.ACV[i] = insert ? newACV : layerACV;
        oitData.D[i] = insert ? newD : layerD;
        //Pop Current Layer And Insert It Later
        newACV = insert ? layerACV : newACV;
        newD = insert ? layerD : newD;
    }
    
    //Merge
    vec4 lastACV = oitData.ACV[lastLayer];
    float lastD = oitData.D[lastLayer];
    
    bool newDepthFirst = (newD <= lastD); // At this point, newD points to the last Layer in the original KBuffer (currently popped)
    
    vec4 firstACV = newDepthFirst ? newACV : lastACV;
    float firstD = newDepthFirst ? newD : lastD;
    vec4 secondACV = newDepthFirst ? lastACV : newACV;
    
    oitData.ACV[lastLayer] = vec4(firstACV.rgb + secondACV.rgb * firstACV.a, firstACV.a*secondACV.a);
    oitData.D[lastLayer] = firstD;
    
    //Write KBuffer
    imageStore(mlabAccum0, pixelCoord,  oitData.ACV[0]);
    imageStore(mlabAccum1, pixelCoord,  oitData.ACV[1]);
    imageStore(mlabAccum2, pixelCoord,  oitData.ACV[2]);
    imageStore(mlabAccum3, pixelCoord,  oitData.ACV[3]);
    
    imageStore(mlabDepth, pixelCoord, vec4(oitData.D[0], oitData.D[1], oitData.D[2], oitData.D[3]));
    #endif
}
