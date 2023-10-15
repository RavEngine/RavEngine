#if RGL_WEBGPU_AVAILABLE
#include "RGLWG.hpp"
#include "WGRenderPass.hpp"

namespace RGL{
    WGPUInstance instance; 

    void InitWebGPU(const RGL::InitOptions& options){
        Assert(CanInitAPI(RGL::API::WebGPU), "WebGPU cannot be initialized on this platform.");
        RGL::currentAPI = API::WebGPU;

        WGPUInstanceDescriptor desc {
            .nextInChain = nullptr
        };

        // see: https://github.com/emscripten-core/emscripten/issues/19349
        instance = wgpuCreateInstance(&desc);
    }

    void DeinitWebGPU(){
        wgpuInstanceRelease(instance);
    }

    RGLRenderPassPtr CreateRenderPassWG(const RenderPassConfig& config){
        return std::make_shared<RenderPassWG>(config);
    }
}

#endif