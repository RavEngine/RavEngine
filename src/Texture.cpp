#include "Texture.hpp"
#include <bimg/bimg.h>
#include <bx/bx.h>
#include <bx/readerwriter.h>
#include "App.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Debug.hpp"

using namespace std;
using namespace RavEngine;

Ref<RuntimeTexture> TextureManager::defaultTexture;

Texture::Texture(const std::string& name){
	//read from resource
	
	auto data = App::Resources->FileContentsAt(("/textures/" + name).c_str());
	
	int width, height,channels;
	stbi_uc const* datastr = reinterpret_cast<const unsigned char* const>(data.c_str());
	auto compressed_size = sizeof(stbi_uc) * data.size();
	
	unsigned char* bytes = stbi_load_from_memory(datastr, compressed_size, &width, &height, &channels, 4);
	if (bytes == nullptr){
		Debug::Fatal("Cannot load texture: {}",stbi_failure_reason());
	}
	
	bool hasMipMaps = false;
	uint16_t numlayers = 1;
	
	CreateTexture(width, height, hasMipMaps, numlayers, channels, bytes);
	
}

void Texture::Bind(int id, const SamplerUniform &uniform){
	bgfx::setTexture(id, uniform, texture);
}

void Texture::CreateTexture(int width, int height, bool hasMipMaps, int numlayers, uint16_t numChannels, const uint8_t *data, int flags){
	auto format = bgfx::TextureFormat::RGBA8;

	auto uncompressed_size = width * height * numChannels * numlayers;
	const bgfx::Memory* textureData = (data == nullptr) ? nullptr : bgfx::copy(data, uncompressed_size);
	texture = bgfx::createTexture2D(width,height,hasMipMaps,numlayers,format,flags,textureData);
	
	if(!bgfx::isValid(texture)){
		Debug::Fatal("Cannot create texture");
	}
}
