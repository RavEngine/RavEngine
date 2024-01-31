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
    
    MTLTextureDescriptor* desc;
    if (config.isCubemap){
        desc = [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:format size:config.width mipmapped:config.mipLevels > 1];
    }
    else{
        desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format width:config.width height:config.height mipmapped:config.mipLevels > 1];
    }
    
    desc.mipmapLevelCount = config.mipLevels;
    auto usage = rgl2mtlTextureUsage(config.usage);
    desc.usage = usage;
    desc.storageMode = (config.aspect.HasDepth || config.aspect.HasStencil) ? MTLStorageModePrivate :
#if TARGET_OS_IPHONE
    MTLStorageModeShared;
#else
    MTLStorageModeManaged;
#endif
    texture = [owningDevice->device newTextureWithDescriptor:desc];
    
    mipTextures.reserve(config.mipLevels - 1);
    for(int i = 1; i < config.mipLevels; i++){
        auto tex = [texture newTextureViewWithPixelFormat:format textureType:MTLTextureType2D levels:NSMakeRange(i, 1) slices:NSMakeRange(0, 1)];
        mipTextures.push_back(tex);
    }
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

TextureView TextureMTL::GetDefaultView() const{
    return TextureView::NativeHandles::mtl_t{this, TextureView::NativeHandles::mtl_t::ALL_MIPS};
}
TextureView TextureMTL::GetViewForMip(uint32_t mip) const{
    if (mip == 0){
        return GetDefaultView();
    }
    
    return TextureView::NativeHandles::mtl_t{this, mip};
}

RGLCustomTextureViewPtr TextureMTL::MakeCustomTextureView(const CustomTextureViewConfig& config) const
{
	return RGLCustomTextureViewPtr();
}

id<MTLTexture> TextureMTL::ViewToTexture(const TextureView& view){
    if (view.texture.mtl.mip == TextureView::NativeHandles::mtl_t::ALL_MIPS){
        return view.texture.mtl.texture->texture;
    }
    else{
        return view.texture.mtl.texture->mipTextures.at(view.texture.mtl.mip - 1);
    }
}

}
#endif
