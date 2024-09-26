#if !RVE_SERVER
#include "Texture.hpp"
#include "App.hpp"
//#define STB_IMAGE_IMPLEMENTATION // don't define here, because rlottie & bimg define them
#include <stb_image.h>
#include "Debug.hpp"
#include <lunasvg.h>
#include "Filesystem.hpp"
#include "VirtualFileSystem.hpp"
#if !RVE_SERVER
#include <RGL/TextureFormat.hpp>
#include "RenderEngine.hpp"
#include <RGL/Device.hpp>
#include <RGL/Texture.hpp>
#endif

using namespace std;
using namespace RavEngine;

STATIC(Texture::Manager::defaultTexture);
STATIC(Texture::Manager::defaultNormalTexture);
STATIC(Texture::Manager::zeroTexture);

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
	GetApp()->GetResources().FileContentsAt(Format("/textures/{}", name).c_str(),data);
    
    // load the SVG
    auto document = lunasvg::Document::loadFromData(reinterpret_cast<char*>(data.data()), data.size());
    auto bitmap = document->renderToBitmap(width,height);
    
    // give to backend
    CreateTexture(width, height, { 
        .mipLevels = 1, 
        .numLayers = 1, 
        .initialData = bitmap.data() 
    });
}

RavEngine::Texture::Texture(const Filesystem::Path& pathOnDisk)
{
	int width, height, channels;
	unsigned char* bytes = stbi_load(pathOnDisk.string().c_str(), &width, &height, &channels, 4);
	if (bytes == nullptr) {
		Debug::Fatal("Cannot load texture from disk {}: {}", pathOnDisk.string().c_str(), stbi_failure_reason());
	}

    CreateTexture(width, height, {
        .mipLevels = 1,
        .numLayers = 1,
        .initialData = bytes
     });
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
	
	uint16_t numlayers = 1;
	
    CreateTexture(width, height, {
        .mipLevels = 1, 
        .numLayers = 1,
        .initialData = bytes
    });
	stbi_image_free(bytes);
	
}


void Texture::CreateTexture(int width, int height, const Config& config){

	uint16_t numChannels = 4;	//TODO: allow n-channel textures
	RGL::TextureFormat format = config.format;
	
	uint32_t uncompressed_size = width * height * numChannels * config.numLayers;

	auto device = GetApp()->GetDevice();
    if (config.initialData != nullptr){
        texture = device->CreateTextureWithData({
            .usage = {.TransferDestination = true, .Sampled = true, .ColorAttachment = config.enableRenderTarget},
            .aspect = {.HasColor = true},
            .width = uint32_t(width),
            .height = uint32_t(height),
            .mipLevels = config.mipLevels,
            .format = format,
            .debugName = config.debugName
        }, { config.initialData,uncompressed_size });
    }
    else{
        texture = device->CreateTexture({
            .usage = {.TransferDestination = true, .Sampled = true, .ColorAttachment = config.enableRenderTarget},
            .aspect = {.HasColor = true},
            .width = uint32_t(width),
            .height = uint32_t(height),
            .mipLevels = config.mipLevels,
            .format = format,
            .debugName = config.debugName,
        });
    }
}

Texture::~Texture() {
	if (auto app = GetApp()) {
        if (app->HasRenderEngine()){
            app->GetRenderEngine().gcTextures.enqueue(texture);
        }
	}
}

RenderTexture::RenderTexture(int width, int height){
    collection = GetApp()->GetRenderEngine().CreateRenderTargetCollection({ static_cast<unsigned int>(width), static_cast<unsigned int>(height) });
    finalFB = New<RuntimeTexture>(width, height, Texture::Config{.enableRenderTarget = true, .format = RGL::TextureFormat::BGRA8_Unorm, .debugName="Render Texture"});
    collection.finalFramebuffer = finalFB->GetRHITexturePointer().get();
}

Ref<RuntimeTexture> RenderTexture::GetTexture(){
    return finalFB;
}
#endif
