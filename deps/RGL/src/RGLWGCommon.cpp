#if RGL_WEBGPU_AVAILABLE
#include "RGLWG.hpp"

namespace RGL{
    WGPUInstance instance; 

    void InitWebGPU(const RGL::InitOptions& options){
        Assert(CanInitAPI(RGL::API::WebGPU), "WebGPU cannot be initialized on this platform.");
        RGL::currentAPI = API::WebGPU;

        WGPUInstanceDescriptor desc {
            .nextInChain = nullptr
        };

        instance = wgpuCreateInstance(&desc);
    }

    void DeinitWebGPU(){
        wgpuInstanceRelease(instance);
    }

}

#endif