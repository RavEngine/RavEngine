#if RGL_WEBGPU_AVAILABLE
#include "WGTexture.hpp"
#include <iostream>

namespace RGL{
TextureWG::~TextureWG(){
    if (owning){
        wgpuTextureViewRelease(texture);
    }
}

TextureWG::TextureWG(decltype(texture) texture, const Dimension& dim, bool) : texture(texture), ITexture(dim), owning(owning){

}
TextureWG::TextureWG(const std::shared_ptr<DeviceWG> owningDevice, const TextureConfig& config, const untyped_span data) : TextureWG(owningDevice, config){

}
TextureWG::TextureWG(const std::shared_ptr<DeviceWG> owningDevice, const TextureConfig& config) : TextureWG(nullptr, {config.width,config.height}){

}

Dimension TextureWG::GetSize() const{
    return size;
}

TextureView TextureWG::GetDefaultView() const{
    return TextureView{texture};
}
TextureView TextureWG::GetViewForMip(uint32_t mip) const{
    return TextureView{mipViews.at(mip)};
}

RGLCustomTextureViewPtr TextureWG::MakeCustomTextureView(const CustomTextureViewConfig& config) const{
    return RGLCustomTextureViewPtr{};
}

}

#endif
