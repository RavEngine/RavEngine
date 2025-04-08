#extension GL_ARB_fragment_shader_interlock : enable

#define VARYINGDIR in
#include "mesh_varyings.glsl"

#if RVE_TRANSPARENT
#if RVE_TBDR
    layout(input_attachment_index = 0, binding = 23) uniform subpassInput mlabAccum0;
    layout(input_attachment_index = 1, binding = 24) uniform subpassInput mlabAccum1;
    layout(input_attachment_index = 2, binding = 25) uniform subpassInput mlabAccum2;
    layout(input_attachment_index = 3, binding = 26) uniform subpassInput mlabAccum3;
    layout(input_attachment_index = 4, binding = 27) uniform subpassInput mlabDepth;

    layout(location = 0) out vec4 mlabAccum0Out;
    layout(location = 1) out vec4 mlabAccum1Out;
    layout(location = 2) out vec4 mlabAccum2Out;
    layout(location = 3) out vec4 mlabAccum3Out;
    layout(location = 4) out vec4 mlabDepthOut;
#else
    layout(binding = 23, rgba16f) uniform image2D mlabAccum0;
    layout(binding = 24, rgba8) uniform image2D mlabAccum1;
    layout(binding = 25, rgba8) uniform image2D mlabAccum2;
    layout(binding = 26, rgba8) uniform image2D mlabAccum3;
    layout(binding = 27, rgba16f) uniform image2D mlabDepth;
#endif
layout(early_fragment_tests) in;
#else
    #if !RVE_DEPTHONLY 
    layout(location = 0) out vec4 result;
    #endif
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
    const vec4 depthValues
    #if RVE_TBDR
        = subpassLoad(mlabDepth);
        oitData.ACV[0] = subpassLoad(mlabAccum0);
        oitData.ACV[1] = subpassLoad(mlabAccum1);
        oitData.ACV[2] = subpassLoad(mlabAccum2);
        oitData.ACV[3] = subpassLoad(mlabAccum3);
    #else
        = imageLoad(mlabDepth, pixelCoord);
        oitData.ACV[0] = imageLoad(mlabAccum0, pixelCoord);
        oitData.ACV[1] = imageLoad(mlabAccum1, pixelCoord);
        oitData.ACV[2] = imageLoad(mlabAccum2, pixelCoord);
        oitData.ACV[3] = imageLoad(mlabAccum3, pixelCoord);
    #endif
    
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
    
    const vec4 dvec = vec4(oitData.D[0], oitData.D[1], oitData.D[2], oitData.D[3]);
    
    //Write KBuffer
    #if RVE_TBDR
        mlabAccum0Out = oitData.ACV[0];
        mlabAccum1Out = oitData.ACV[1];
        mlabAccum2Out = oitData.ACV[2];
        mlabAccum3Out = oitData.ACV[3];
        mlabDepthOut = dvec;
    #else
        imageStore(mlabAccum0, pixelCoord,  oitData.ACV[0]);
        imageStore(mlabAccum1, pixelCoord,  oitData.ACV[1]);
        imageStore(mlabAccum2, pixelCoord,  oitData.ACV[2]);
        imageStore(mlabAccum3, pixelCoord,  oitData.ACV[3]);
        
        imageStore(mlabDepth, pixelCoord, dvec);
    #endif
    #endif
}
