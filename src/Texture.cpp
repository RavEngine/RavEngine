#include "Texture.hpp"
#include "App.hpp"
//#define STB_IMAGE_IMPLEMENTATION // don't define here, because rlottie & bimg define them
#include <stb_image.h>
#include "Debug.hpp"
#include <lunasvg.h>
#include "Filesystem.hpp"
#include "VirtualFileSystem.hpp"
#include <RGL/TextureFormat.hpp>
#include "RenderEngine.hpp"
#include <RGL/Device.hpp>
#include <RGL/Texture.hpp>

using namespace std;
using namespace RavEngine;

STATIC(Texture::Manager::defaultTexture);
STATIC(Texture::Manager::defaultNormalTexture);

inline static bool IsRasterImage(const std::string& filepath){
    // assume that anything not in the whitelist is a raster image
    if (Filesystem::Path(filepath).extension() == ".svg"){
        return false;
    }
    return true;
}

Texture::Texture(const std::string& name, uint16_t width, uint16_t height){
    Debug::Assert(!IsRasterImage(name), "This texture constructor only allows vector image formats");
    
    RavEngine::Vector<uint8_t> data;
	GetApp()->GetResources().FileContentsAt(StrFormat("/textures/{}", name).c_str(),data);
    
    // load the SVG
    auto document = lunasvg::Document::loadFromData(reinterpret_cast<char*>(data.data()), data.size());
    auto bitmap = document->renderToBitmap(width,height);
    
    // give to BGFX
    CreateTexture(width, height, false, 1, bitmap.data());
}

RavEngine::Texture::Texture(const Filesystem::Path& pathOnDisk)
{
	int width, height, channels;
	unsigned char* bytes = stbi_load(pathOnDisk.string().c_str(), &width, &height, &channels, 4);
	if (bytes == nullptr) {
		Debug::Fatal("Cannot load texture from disk {}: {}", pathOnDisk.string().c_str(), stbi_failure_reason());
	}

	CreateTexture(width,height,false,1,bytes);
	stbi_image_free(bytes);
}

Texture::Texture(const std::string& name){
    Debug::Assert(IsRasterImage(name), "This texture constructor only allows raster image formats");
    
	//read from resource
	
    RavEngine::Vector<uint8_t> data;
	GetApp()->GetResources().FileContentsAt(("/textures/" + name).c_str(),data);
	
	int width, height,channels;
	auto compressed_size = sizeof(stbi_uc) * data.size();
	
    unsigned char* bytes = stbi_load_from_memory(&data[0], Debug::AssertSize<int>(compressed_size), &width, &height, &channels, 4);
	if (bytes == nullptr){
		Debug::Fatal("Cannot load texture: {}",stbi_failure_reason());
	}
	
	bool hasMipMaps = false;
	uint16_t numlayers = 1;
	
	CreateTexture(width, height, hasMipMaps, numlayers, bytes);
	stbi_image_free(bytes);
	
}


void Texture::CreateTexture(int width, int height, bool hasMipMaps, int numlayers, const uint8_t *data, int flags){

	uint16_t numChannels = 4;	//TODO: allow n-channel textures
	RGL::TextureFormat format = RGL::TextureFormat::RGBA8_Unorm;
	
	uint32_t uncompressed_size = width * height * numChannels * numlayers;

	auto device = GetApp()->GetDevice();
	texture = device->CreateTextureWithData({
		.usage = {.TransferDestination = true, .Sampled = true},
		.aspect = {.HasColor = true},
		.width = uint32_t(width),
		.height = uint32_t(height),
		.format = format
		}, { data,uncompressed_size });
}

Texture::~Texture() {
	if (auto app = GetApp()) {
		app->GetRenderEngine().gcTextures.enqueue(texture);
	}
}