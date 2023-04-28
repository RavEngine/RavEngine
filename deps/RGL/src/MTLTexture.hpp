#pragma once
#include <RGL/Types.hpp>
#include <RGL/Texture.hpp>
#include "MTLObjCCompatLayer.hpp"
#include <RGL/Span.hpp>
#include <memory>

namespace RGL{
struct DeviceMTL;

struct TextureMTL : public ITexture{
    OBJC_ID(CAMetalDrawable) drawable = nullptr;
    OBJC_ID(MTLTexture) texture = nullptr;
    
    // default constructor, don't explicity use
    TextureMTL() : ITexture({0,0}){}
    virtual ~TextureMTL();
    
    TextureMTL(decltype(drawable), const Dimension&);
    TextureMTL(const std::shared_ptr<DeviceMTL>, const TextureConfig& config, const untyped_span);
    TextureMTL(const std::shared_ptr<DeviceMTL>, const TextureConfig& config);
    
    Dimension GetSize() const;
};

}
