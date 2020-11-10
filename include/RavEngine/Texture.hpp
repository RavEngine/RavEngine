#pragma once
#include "SharedObject.hpp"
#include <bgfx/bgfx.h>
#include "Uniform.hpp"

namespace RavEngine{

class Texture : public SharedObject{
public:
	Texture(const std::string& filename);
	
	virtual ~Texture(){
		bgfx::destroy(texture);
	}
	
	void Bind(int id, const SamplerUniform& uniform);
	
protected:
	bgfx::TextureHandle texture = BGFX_INVALID_HANDLE;
	Texture(){}
	void CreateTexture(int width, int height, bool hasMipMaps, int numlayers, uint16_t numChannels, const uint8_t *data, int flags = BGFX_TEXTURE_SRGB | BGFX_SAMPLER_POINT);
};

class RuntimeTexture : public Texture{
public:
	RuntimeTexture(const std::string& filename) = delete;
	RuntimeTexture(int width, int height, bool hasMipMaps, int numlayers, uint16_t numChannels, const uint8_t *data, int flags = BGFX_TEXTURE_SRGB | BGFX_SAMPLER_POINT) : Texture(){
		CreateTexture(width, height, hasMipMaps, numlayers, numChannels, nullptr);
	}
};

class TextureManager{
public:
	static Ref<RuntimeTexture> defaultTexture;
};

}
