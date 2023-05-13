#if RGL_WEBGPU_AVAILABLE
#include "WGDevice.hpp"
#include "RGLCommon.hpp"
#include "RGLWG.hpp"
#include "WGCommandQueue.hpp"
#include <cassert>
#include <format>

namespace RGL{
    /**
     * Utility function to get a WebGPU adapter, so that
     *     WGPUAdapter adapter = requestAdapter(options);
     * is roughly equivalent to
     *     const adapter = await navigator.gpu.requestAdapter(options);
     */
    WGPUAdapter requestAdapter(WGPUInstance instance, WGPURequestAdapterOptions const * options) {
        // A simple structure holding the local information shared with the
        // onAdapterRequestEnded callback.
        struct UserData {
            WGPUAdapter adapter = nullptr;
            bool requestEnded = false;
        };
        UserData userData;

        // Callback called by wgpuInstanceRequestAdapter when the request returns
        // This is a C++ lambda function, but could be any function defined in the
        // global scope. It must be non-capturing (the brackets [] are empty) so
        // that it behaves like a regular C function pointer, which is what
        // wgpuInstanceRequestAdapter expects (WebGPU being a C API). The workaround
        // is to convey what we want to capture through the pUserData pointer,
        // provided as the last argument of wgpuInstanceRequestAdapter and received
        // by the callback as its last argument.
        auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const * message, void * pUserData) {
            UserData& userData = *reinterpret_cast<UserData*>(pUserData);
            if (status == WGPURequestAdapterStatus_Success) {
                userData.adapter = adapter;
            } else {
                FatalError(std::string("Could not get WebGPU adapter: ") + message);
            }
            userData.requestEnded = true;
        };

        // Call to the WebGPU request adapter procedure
        wgpuInstanceRequestAdapter(
            instance /* equivalent of navigator.gpu */,
            options,
            onAdapterRequestEnded,
            (void*)&userData
        );

        // In theory we should wait until onAdapterReady has been called, which
        // could take some time (what the 'await' keyword does in the JavaScript
        // code). In practice, we know that when the wgpuInstanceRequestAdapter()
        // function returns its callback has been called.
        assert(userData.requestEnded);

        return userData.adapter;
    }

    /**
     * Utility function to get a WebGPU device, so that
     *     WGPUAdapter device = requestDevice(adapter, options);
     * is roughly equivalent to
     *     const device = await adapter.requestDevice(descriptor);
     * It is very similar to requestAdapter
     */
    WGPUDevice requestDevice(WGPUAdapter adapter, WGPUDeviceDescriptor const * descriptor) {
        struct UserData {
            WGPUDevice device = nullptr;
            bool requestEnded = false;
        };
        UserData userData;

        auto onDeviceRequestEnded = [](WGPURequestDeviceStatus status, WGPUDevice device, char const * message, void * pUserData) {
            UserData& userData = *reinterpret_cast<UserData*>(pUserData);
            if (status == WGPURequestDeviceStatus_Success) {
                userData.device = device;
            } else {
                FatalError(std::string( "Could not get WebGPU adapter: " ) + message);
            }
            userData.requestEnded = true;
        };

        wgpuAdapterRequestDevice(
            adapter,
            descriptor,
            onDeviceRequestEnded,
            (void*)&userData
        );

        assert(userData.requestEnded);

        return userData.device;
    }


    RGLDevicePtr CreateDefaultDeviceWG(){
        return std::make_shared<DeviceWG>();
    }

    DeviceWG::DeviceWG() {
        WGPURequestAdapterOptions adapterOpts{};
        adapter = requestAdapter(instance,&adapterOpts);

        WGPUDeviceDescriptor deviceDesc{
            .nextInChain = nullptr,
            .label = "RGL WGPU device",
            .requiredFeaturesCount = 0,
            .defaultQueue = {
                .nextInChain = nullptr,
                .label = "RGL Default queue"
            }
        };
        device = requestDevice(adapter,&deviceDesc);
        auto onDeviceError = [](WGPUErrorType type, char const* message, void* /* pUserData */) {
            FatalError(std::string("WGPU Device Error: ") + message);
        };
        wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError, nullptr /* pUserData */);
    }

    DeviceWG::~DeviceWG() {
        wgpuAdapterRelease(adapter);
        wgpuDeviceRelease(device);
    }

std::string DeviceWG::GetBrandString() {
   return "Unknown WebGPU device";
}

RGLSwapchainPtr DeviceWG::CreateSwapchain(RGLSurfacePtr isurface, RGLCommandQueuePtr presentQueue, int width, int height){
  
}

RGLPipelineLayoutPtr DeviceWG::CreatePipelineLayout(const PipelineLayoutDescriptor& desc) {
}

RGLRenderPipelinePtr DeviceWG::CreateRenderPipeline(const RenderPipelineDescriptor& desc) {
}

RGLComputePipelinePtr DeviceWG::CreateComputePipeline(const RGL::ComputePipelineDescriptor& desc) {
}

RGLShaderLibraryPtr DeviceWG::CreateDefaultShaderLibrary() {
    FatalError("CreateDefaultShaderLibrary not implemented");
}

RGLShaderLibraryPtr DeviceWG::CreateShaderLibraryFromName(const std::string_view& name){
}

RGLShaderLibraryPtr DeviceWG::CreateShaderLibraryFromBytes(const std::span<uint8_t>) {
    FatalError("Not Implemented");
}

RGLShaderLibraryPtr DeviceWG::CreateShaderLibrarySourceCode(const std::string_view source, const FromSourceConfig& config) {
}

RGLShaderLibraryPtr DeviceWG::CreateShaderLibraryFromPath(const std::filesystem::path&) {
    FatalError("ShaderLibraryMTL");
}

RGLBufferPtr DeviceWG::CreateBuffer(const BufferConfig& config) {
}

RGLCommandQueuePtr DeviceWG::CreateCommandQueue(QueueType type) {
}

RGLFencePtr DeviceWG::CreateFence(bool preSignaled) {
}

RGLTexturePtr DeviceWG::CreateTextureWithData(const TextureConfig& config, untyped_span data){
}

RGLTexturePtr DeviceWG::CreateTexture(const TextureConfig& config){
}

RGLSamplerPtr DeviceWG::CreateSampler(const SamplerConfig& config){
}

void DeviceWG::BlockUntilIdle() {
    
}

RGL::DeviceData DeviceWG::GetDeviceData() {
    return {
       
    };
}

}

#endif