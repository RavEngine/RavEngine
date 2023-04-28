#if RGL_MTL_AVAILABLE
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include "MTLTexture.hpp"
#include "MTLDevice.hpp"
#include "RGLMTL.hpp"

namespace RGL {

TextureMTL::TextureMTL(decltype(drawable) texture, const Dimension& size) : drawable(texture), ITexture(size){
    
}

TextureMTL::~TextureMTL(){
//    [drawable release];
//    [texture release];
}

TextureMTL::TextureMTL(const std::shared_ptr<DeviceMTL> owningDevice, const TextureConfig& config) : TextureMTL(nullptr, {config.width,config.height})
{
    MTLPixelFormat format = rgl2mtlformat(config.format);
    
    auto desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format width:config.width height:config.height mipmapped:config.mipLevels > 1];
    auto usage = rgl2mtlTextureUsage(config.usage);;
    desc.usage = usage;
    desc.storageMode = (config.aspect.HasDepth || config.aspect.HasStencil) ? MTLStorageModePrivate :
#if TARGET_OS_IPHONE
    MTLStorageModeShared;
#else
    MTLStorageModeManaged;
#endif
    texture = [owningDevice->device newTextureWithDescriptor:desc];
}

Dimension TextureMTL::GetSize() const{
    return size;
}

TextureMTL::TextureMTL(const std::shared_ptr<DeviceMTL> owningDevice, const TextureConfig& config, const untyped_span data) : TextureMTL(owningDevice, config){
    
    
    MTLRegion region = {
        { 0, 0, 0 },                   // MTLOrigin
        {config.width, config.height, 1} // MTLSize
    };
    
    NSUInteger bytesPerRow = 4 * config.width;   // TODO: replace 4 with nchannels of format
    
    [texture replaceRegion:region
                mipmapLevel:0
                  withBytes:data.data()
                bytesPerRow:bytesPerRow];
}

}
#endif
