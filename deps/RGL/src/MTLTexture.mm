#if RGL_MTL_AVAILABLE
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include "MTLTexture.hpp"
#include "MTLDevice.hpp"
#include "RGLMTL.hpp"

namespace RGL {

uint32_t bytesPerPixel(RGL::TextureFormat format){
    switch(format){
        case decltype(format)::RGBA32_Sfloat:   return sizeof(float) * 4;
        case decltype(format)::RGBA16_Snorm:
        case decltype(format)::RGBA16_Unorm:
        case decltype(format)::RGBA16_Sfloat:   return sizeof(float) / 2 * 4;
        case decltype(format)::RGBA8_Uint:
        case decltype(format)::RGBA8_Unorm:   return sizeof(uint8_t) * 4;
        default:
            FatalError("bytesPerPixel: Invalid textureformat");
    }
}

TextureMTL::TextureMTL(decltype(drawable) texture, const Dimension& size) : drawable(texture), ITexture(size){
    
}

TextureMTL::~TextureMTL(){
//    [drawable release];
//    [texture release];
    if (owningDevice){
        owningDevice->textureFreelist.Deallocate(globalIndex);
    }
}

TextureMTL::TextureMTL(const std::shared_ptr<DeviceMTL> owningDevice, const TextureConfig& config) : TextureMTL(nullptr, {config.width,config.height})
{
    this->owningDevice = owningDevice;
    
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
    desc.storageMode = (config.aspect.HasDepth || config.aspect.HasStencil || config.usage.ColorAttachment) ? MTLStorageModePrivate :
#if TARGET_OS_IPHONE
    MTLStorageModeShared;
#else
    MTLStorageModeManaged;
#endif
    texture = [owningDevice->device newTextureWithDescriptor:desc];
    
    if (config.debugName.data() != nullptr){
        [texture setLabel:[NSString stringWithUTF8String:config.debugName.data()]];
    }
    
    mipTextures.reserve(config.mipLevels - 1);
    for(int i = 1; i < config.mipLevels; i++){
        auto tex = [texture newTextureViewWithPixelFormat:format textureType:MTLTextureType2D levels:NSMakeRange(i, 1) slices:NSMakeRange(0, 1)];
        mipTextures.push_back(tex);
    }
    
    // add to the bindless heap
    globalIndex = owningDevice->textureFreelist.Allocate();
    [owningDevice->globalTextureEncoder setTexture:texture atIndex:globalIndex];
}

Dimension TextureMTL::GetSize() const{
    return size;
}

TextureMTL::TextureMTL(const std::shared_ptr<DeviceMTL> owningDevice, const TextureConfig& config, const TextureUploadData& data) : TextureMTL(owningDevice, config){
    
    
    MTLRegion region = {
        { 0, 0, 0 },                   // MTLOrigin
        {config.width, config.height, 1} // MTLSize
    };
    
    NSUInteger bytesPerRow = bytesPerPixel(config.format) * config.width;
    
    [texture replaceRegion:region
                mipmapLevel:0
                  withBytes:data.data.data()
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

TextureView RGL::CustomTextureViewMTL::GetView() const
{
    return TextureView();
}

}
#endif
