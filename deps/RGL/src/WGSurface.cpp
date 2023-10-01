#if RGL_WEBGPU_AVAILABLE
#include "WGSurface.hpp"
#include "RGLWG.hpp"
#include <iostream>

namespace RGL{


    SurfaceWG::SurfaceWG(const void* pointer){
        // we are expecting a CSS selector string, like "#canvas", for the value of pointer
        WGPUSurfaceDescriptorFromCanvasHTMLSelector canvasDesc{
            .chain = {
                .next = nullptr,
                .sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector 
            },
            .selector = static_cast<const char* const>(pointer)
        };
        WGPUSurfaceDescriptor desc{
            .nextInChain = &canvasDesc.chain,
            .label = "Surface"
        };
        surface = wgpuInstanceCreateSurface(instance, &desc);
    }

    SurfaceWG::~SurfaceWG(){
        wgpuSurfaceRelease(surface);
    }


    RGLSurfacePtr CreateWGSurfaceFromPlatformHandle(const void* pointer){
        return std::make_shared<SurfaceWG>(pointer);
    }
}
#endif