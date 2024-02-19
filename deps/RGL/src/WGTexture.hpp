#pragma once
#include <RGL/Types.hpp>
#include <RGL/Texture.hpp>
#include <RGL/Span.hpp>
#include <memory>
#include <emscripten/html5_webgpu.h>
#include <vector>

namespace RGL{
struct DeviceWG;

struct TextureWG : public ITexture{
    WGPUTextureView texture;
    bool owning = true;
    // default constructor, don't explicity use
    TextureWG() : ITexture({0,0}){}
    virtual ~TextureWG();
    
    TextureWG(decltype(texture), const Dimension&, bool owning = false);
    TextureWG(const std::shared_ptr<DeviceWG>, const TextureConfig& config, const untyped_span);
    TextureWG(const std::shared_ptr<DeviceWG>, const TextureConfig& config);
    
    TextureView GetDefaultView() const final;
	TextureView GetViewForMip(uint32_t mip) const final;
    virtual RGLCustomTextureViewPtr MakeCustomTextureView(const CustomTextureViewConfig& config) const;

    std::vector<WGPUTextureView> mipViews;

    Dimension GetSize() const;
};

}
