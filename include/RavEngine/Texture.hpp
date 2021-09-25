#pragma once
#include <bgfx/bgfx.h>
#include "Uniform.hpp"
#include "Ref.hpp"
#include "Manager.hpp"

namespace RavEngine{

class Texture {
public:
	/**
	 Create a texture given a file
	 @param filename name of the texture
	 */
	Texture(const std::string& filename);
    
    Texture(const std::string& filename, uint16_t width, uint16_t height);
	
	virtual ~Texture(){
		bgfx::destroy(texture);
	}
	
    constexpr inline bgfx::TextureHandle GetTextureHandle() const{
		return texture;
	}
	
	void Bind(int id, const SamplerUniform& uniform);
    
    /**
     Use the manager to avoid loading duplicate textures
     Works with Runtime textures as well, using the construction arguments to differentiate textures
     */
    struct Manager : public GenericWeakManager<std::string,Texture>{};
	
protected:
	bgfx::TextureHandle texture = BGFX_INVALID_HANDLE;
	Texture(){}
	
	void CreateTexture(int width, int height, bool hasMipMaps, int numlayers, const uint8_t *data, int flags = BGFX_TEXTURE_SRGB | BGFX_SAMPLER_POINT);
};

class RuntimeTexture : public Texture{
public:
	RuntimeTexture(const std::string& filename) = delete;
	
	/**
	 Create a texture from data
	 @param width width of the texture
	 @param height height of the texture
	 @param hasMipMaps does the texture contain mip maps
	 @param numLayers the number of layers in the texture (NOT channels!)
	 @param data pointer to the image data. Must be a 4-channel image.
	 @param flags optional creation flags
	 */
	RuntimeTexture(int width, int height, bool hasMipMaps, int numlayers, const uint8_t *data, int flags = BGFX_TEXTURE_SRGB | BGFX_SAMPLER_POINT) : Texture(){
		CreateTexture(width, height, hasMipMaps, numlayers, data,flags);
	}
};

class TextureManager{
public:
	static Ref<RuntimeTexture> defaultTexture;
};

}
