#if RGL_WEBGPU_AVAILABLE
#include "WGSwapchain.hpp"
#include "WGDevice.hpp"
#include <emscripten/html5_webgpu.h>
#include "RGLCommon.hpp"

namespace RGL{
    SwapchainWG::SwapchainWG(decltype(surface) surface, uint32_t width, uint32_t height, const std::shared_ptr<DeviceWG> owningDevice){
        WGPUSwapChainDescriptor swapChainDesc{
            .nextInChain = nullptr,
            .width = width,
            .height = height,
            .format = wgpuSurfaceGetPreferredFormat(surface->surface,owningDevice->adapter),
            .usage = WGPUTextureUsage_RenderAttachment,
            .presentMode = WGPUPresentMode_Fifo,
        };
        swapchain = wgpuDeviceCreateSwapChain(owningDevice->device, surface->surface, &swapChainDesc);
    }

    SwapchainWG::~SwapchainWG(){
        wgpuSwapChainRelease(swapchain);
    }

    void SwapchainWG::Resize(uint32_t width, uint32_t height){
        FatalError("Resize: not implemented");
    }

    void SwapchainWG::GetNextImage(uint32_t* index){
        FatalError("GetNextImage: not implemented");
    }

    ITexture* SwapchainWG::ImageAtIndex(uint32_t index){
        FatalError("ImageAtIndex: not implemented");
        return nullptr;
    }

    void SwapchainWG::Present(const SwapchainPresentConfig&){
        
    }
}

#endif