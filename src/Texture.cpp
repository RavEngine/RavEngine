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
extern "C" {
    #include <dds/dds.h>
}
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
    const char* failureReason = nullptr;
    Function<void()> freer;
    if (bytes != nullptr) {
        freer = [bytes] {stbi_image_free(bytes); };
        goto load;
    }
    else {
        failureReason = stbi_failure_reason();
    }

    
    // try loading as a DDS
    {
        auto dds = dds_load_from_file(pathOnDisk.string().c_str());
        if (dds->pixels != nullptr) {
            bytes = dds->pixels;
            width = dds->header.width;
            height = dds->header.height;
            freer = [dds] {dds_image_free(dds); };
            goto load;
        }
        else{
            failureReason = "Failed to load DDS";
        }
    }

    // if we are here then nothing loaded the image
	if (bytes == nullptr) {
		Debug::Fatal("Cannot load texture from disk {}: {}", pathOnDisk.string().c_str(), failureReason);
	}

    load:
    CreateTexture(width, height, {
        .mipLevels = 1,
        .numLayers = 1,
        .initialData = bytes
     });
    freer();
}

Texture::Texture(const std::string& name){
    Debug::Assert(IsRasterImage(name), "This texture constructor only allows raster image formats");
    
	//read from resource
	
    RavEngine::Vector<uint8_t> data;
	GetApp()->GetResources().FileContentsAt(("/textures/" + name).c_str(),data);
    std::function<void()> freer;
    const char* failureReason = nullptr;
	
	int width = 0, height = 0,channels;
	auto compressed_size = sizeof(stbi_uc) * data.size();
	
    unsigned char* bytes = stbi_load_from_memory(&data[0], Debug::AssertSize<int>(compressed_size), &width, &height, &channels, 4);
	if (bytes != nullptr){
        freer = [bytes] { stbi_image_free(bytes); };
        goto load;
	}
    else {
        failureReason = stbi_failure_reason();
    }

    // try loading as a DDS
    {
        auto dds = dds_load_from_memory(reinterpret_cast<const char*>(data.data()), data.size());
        if (dds->pixels != nullptr) {
            bytes = dds->pixels;
            width = dds->header.width;
            height = dds->header.height;
            freer = [dds] {dds_image_free(dds); };
            goto load;
        }
        else {
            failureReason = "Failed to load DDS";
        }
    }

    // if we are here then we failed to load the image
    if (bytes == nullptr) {
        Debug::Fatal("Cannot load texture {}: {}", name, failureReason);
    }
	
    load:
	uint16_t numlayers = 1;
	
    CreateTexture(width, height, {
        .mipLevels = 1, 
        .numLayers = 1,
        .initialData = bytes
    });
    freer();
	
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
