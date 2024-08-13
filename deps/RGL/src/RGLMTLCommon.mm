#if RGL_MTL_AVAILABLE
#import <Metal/Metal.h>
#include "RGLMTL.hpp"
#include "RGLCommon.hpp"
#include "MTLRenderPass.hpp"

using namespace RGL;

namespace RGL{

void InitMTL(const RGL::InitOptions&) {
    Assert(CanInitAPI(RGL::API::Metal), "Metal cannot be initialized on this platform.");
    RGL::currentAPI = API::Metal;
}

void DeinitMTL(){
    // do nothing for now
}

MTLPixelFormat rgl2mtlformat(TextureFormat format){
    switch(format){
        case decltype(format)::Undefined: return MTLPixelFormatInvalid;
        case decltype(format)::BGRA8_Unorm: return MTLPixelFormatBGRA8Unorm;
        case decltype(format)::RGBA8_Uint: return MTLPixelFormatRGBA8Uint;
        case decltype(format)::RGBA8_Unorm: return MTLPixelFormatRGBA8Unorm;
        case decltype(format)::D32SFloat: return MTLPixelFormatDepth32Float;
        case decltype(format)::RGBA16_Unorm: return MTLPixelFormatRGBA16Unorm;
        case decltype(format)::RGBA32_Sfloat: return MTLPixelFormatRGBA32Float;
        case decltype(format)::RGBA16_Snorm: return MTLPixelFormatRGBA16Snorm;
        case decltype(format)::RGBA16_Sfloat: return MTLPixelFormatRGBA16Float;
        case decltype(format)::R8_Uint: return MTLPixelFormatR8Uint;
        case decltype(format)::R16_Float: return MTLPixelFormatR16Float;
        case decltype(format)::R32_Uint: return MTLPixelFormatR32Uint;
        case decltype(format)::R32_Float: return MTLPixelFormatR32Float;
#if !TARGET_OS_IPHONE
        case decltype(format)::D24UnormS8Uint: return MTLPixelFormatDepth24Unorm_Stencil8;
#endif
        default:
            FatalError("Texture format not supported");
    }
}

APPLE_API_TYPE(MTLTextureUsage) rgl2mtlTextureUsage(RGL::TextureUsage usage){
    MTLTextureUsage ret = 0;
    if (usage.Sampled){
        ret |= MTLTextureUsageShaderRead;
    }
    
    if (usage.ColorAttachment || usage.DepthStencilAttachment){
        ret |= MTLTextureUsageRenderTarget;
    }
    if (usage.Storage){
        ret |= MTLTextureUsageShaderWrite;
    }
    return ret;
}

RGLRenderPassPtr CreateRenderPassMTL(const RenderPassConfig& config){
    return std::make_shared<RenderPassMTL>(config);
}

}
#endif
