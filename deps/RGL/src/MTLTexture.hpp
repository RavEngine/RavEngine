#pragma once
#include <RGL/Types.hpp>
#include <RGL/Texture.hpp>
#include "MTLObjCCompatLayer.hpp"
#include <RGL/Span.hpp>
#include <memory>
#include <vector>

namespace RGL{
struct DeviceMTL;

struct TextureMTL : public ITexture{
    OBJC_ID(CAMetalDrawable) drawable = nullptr;
    OBJC_ID(MTLTexture) texture = nullptr;
    
    static OBJC_ID(MTLTexture) ViewToTexture(const TextureView& view);
    
    // default constructor, don't explicity use
    TextureMTL() : ITexture({0,0}){}
    virtual ~TextureMTL();
    
    TextureMTL(decltype(drawable), const Dimension&);
    TextureMTL(const std::shared_ptr<DeviceMTL>, const TextureConfig& config, const TextureUploadData&);
    TextureMTL(const std::shared_ptr<DeviceMTL>, const TextureConfig& config);
    
    Dimension GetSize() const;
    
    TextureView GetDefaultView() const final;
    TextureView GetViewForMip(uint32_t mip) const final;

    RGLCustomTextureViewPtr MakeCustomTextureView(const CustomTextureViewConfig& config) const;
    
    uint8_t GetNumMips() const final;
    
    std::vector<OBJC_ID(MTLTexture)> mipTextures;
    
    uint32_t globalIndex = 0;
    
private:
    std::shared_ptr<DeviceMTL> owningDevice;
};

struct CustomTextureViewMTL : public ICustomTextureView {

    TextureView GetView() const;
};

}
