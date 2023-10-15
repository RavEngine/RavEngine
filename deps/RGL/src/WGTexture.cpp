#if RGL_WEBGPU_AVAILABLE
#include "WGTexture.hpp"

namespace RGL{
TextureWG::~TextureWG(){
    wgpuTextureViewRelease(texture);
}

TextureWG::TextureWG(decltype(texture) texture, const Dimension& dim) : texture(texture), ITexture(dim){

}
TextureWG::TextureWG(const std::shared_ptr<DeviceWG> owningDevice, const TextureConfig& config, const untyped_span data) : TextureWG(owningDevice, config){

}
TextureWG::TextureWG(const std::shared_ptr<DeviceWG> owningDevice, const TextureConfig& config) : TextureWG(nullptr, {config.width,config.height}){

}

Dimension TextureWG::GetSize() const{
    return size;
}

}

#endif