#if RGL_WEBGPU_AVAILABLE
#include "WGSwapchain.hpp"
#include "WGDevice.hpp"
#include <emscripten/html5_webgpu.h>
#include <emscripten/html5.h>
#include "RGLCommon.hpp"

namespace RGL{
    WGPUSwapChain SwapchainWG::makeSwapchain(uint32_t width, uint32_t height){
        WGPUSwapChainDescriptor swapChainDesc{
            .nextInChain = nullptr,
            .width = width,
            .height = height,
            .format = wgpuSurfaceGetPreferredFormat(surface->surface,owningDevice->adapter),
            .usage = WGPUTextureUsage_RenderAttachment,
            .presentMode = vsync ? WGPUPresentMode_Fifo : WGPUPresentMode_Immediate,
        };
        return wgpuDeviceCreateSwapChain(owningDevice->device, surface->surface, &swapChainDesc);
    }

    SwapchainWG::SwapchainWG(decltype(surface) surface, uint32_t width, uint32_t height, const std::shared_ptr<DeviceWG> owningDevice) : surface(surface), owningDevice(owningDevice){
        swapchain = makeSwapchain(width, height);
        currentSize = {width, height};
    }

    SwapchainWG::~SwapchainWG(){
        wgpuSwapChainRelease(swapchain);
    }

    void SwapchainWG::Resize(uint32_t width, uint32_t height){
        wgpuSwapChainRelease(swapchain);
        swapchain = makeSwapchain(width, height);
        currentSize = {width, height};
    }

    void SwapchainWG::GetNextImage(uint32_t* index){
        auto next = wgpuSwapChainGetCurrentTextureView(swapchain);

        activeTextures[idx] = TextureWG(next, {static_cast<uint32_t>(currentSize.width), static_cast<uint32_t>(currentSize.height)});

        *index = this->idx;
        idx = (idx + 1) % activeTextures.size();
    }

    ITexture* SwapchainWG::ImageAtIndex(uint32_t index){
       return &activeTextures[index];
    }

    void SwapchainWG::Present(const SwapchainPresentConfig&){
        #if __EMSCRIPTEN__
            emscripten_request_animation_frame([](double time, void* userData){
                return 0;
            },nullptr);
        #else
            wgpuSwapChainPresent(swapchain);
        #endif
    }

    void SwapchainWG::SetVsyncMode(bool mode){
        vsync = mode;
        Resize(currentSize.width, currentSize.height);
    }
}

#endif