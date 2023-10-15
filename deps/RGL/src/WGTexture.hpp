#pragma once
#include <RGL/Types.hpp>
#include <RGL/Texture.hpp>
#include <RGL/Span.hpp>
#include <memory>
#include <emscripten/html5_webgpu.h>

namespace RGL{
struct DeviceWG;

struct TextureWG : public ITexture{
    WGPUTextureView texture;
    // default constructor, don't explicity use
    TextureWG() : ITexture({0,0}){}
    virtual ~TextureWG();
    
    TextureWG(decltype(texture), const Dimension&);
    TextureWG(const std::shared_ptr<DeviceWG>, const TextureConfig& config, const untyped_span);
    TextureWG(const std::shared_ptr<DeviceWG>, const TextureConfig& config);
    
    Dimension GetSize() const;
};

}