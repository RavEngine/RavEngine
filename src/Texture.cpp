#if !RVE_SERVER
#include "Texture.hpp"
#include "Stream.hpp"
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
#include <dds.hpp>
#include <tinyexr.h>

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
        .initialData = {{reinterpret_cast<std::byte*>(bitmap.data()),width * height * 4 * sizeof(float)}}
    });
}

void RavEngine::Texture::InitFromDDS(IStream& stream)
{
   
    std::vector<std::byte> fileData;
    fileData.resize(stream.size());
    stream.read(fileData);

    dds::Image ddsImg;
    auto result = dds::readImage(reinterpret_cast<uint8_t*>(fileData.data()),fileData.size(), &ddsImg);
    if (result != dds::ReadResult::Success) {
        Debug::Fatal("Cannot load DDS: {}", uint32_t(result));
    }

    RGL::TextureFormat dxtFormat = RGL::TextureFormat::Undefined;
    switch (ddsImg.format) {
    case DXGI_FORMAT_BC1_UNORM: dxtFormat = RGL::TextureFormat::BC1_RGBA_Unorm;  break;
    case DXGI_FORMAT_BC3_UNORM: dxtFormat = RGL::TextureFormat::BC3_Unorm; break;
    case DXGI_FORMAT_BC5_UNORM: dxtFormat = RGL::TextureFormat::BC5_Unorm; break;
    default:
        Debug::Fatal("Invalid DDS format: {}", uint32_t(ddsImg.format));
    }

    CreateTexture(ddsImg.width, ddsImg.height, {
        .mipLevels = 1,
        .numLayers = 1,
        .initialData = {{ddsImg.mipmaps[0].data(),ddsImg.mipmaps[0].size()}},
        .format = dxtFormat
    });
}

void RavEngine::Texture::InitFromEXR(IStream& stream) {
    std::vector<std::byte> fileData;
    fileData.resize(stream.size());
    stream.read(fileData);

    int width = 0, height = 0;
    float* rgba = nullptr;
    const char* err = nullptr;
    LoadEXRFromMemory(&rgba,&width,&height,reinterpret_cast<const unsigned char*>(fileData.data()), fileData.size(),&err);

    CreateTexture(width, height, {
       .mipLevels = 1,
       .numLayers = 1,
       .initialData = {{reinterpret_cast<std::byte*>(rgba),width * height * 4 * sizeof(float)}},
        .format = RGL::TextureFormat::RGBA32_Sfloat
    });
    free(rgba);
}

RavEngine::Texture::Texture(const Filesystem::Path& pathOnDisk)
{
    FileStream stream(std::ifstream{ pathOnDisk, std::ios::binary });

    // what kind of texture is this?
    Array<std::byte,16> headerData;
    stream.read(headerData);
    stream.reset();
    if (std::string_view{ reinterpret_cast<const char*>(headerData.data()),4 } == "DDS ") {
        // this is a DDS
        InitFromDDS(stream);
        return;
    }
    if (std::string_view{ reinterpret_cast<const char*>(headerData.data()),2 } == "v/") {
        InitFromEXR(stream);
        return;
    }

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

    // if we are here then nothing loaded the image
	if (bytes == nullptr) {
		Debug::Fatal("Cannot load texture from disk {}: {}", pathOnDisk.string().c_str(), failureReason);
	}

load:
    uint16_t numlayers = 1;
    uint16_t numChannels = 4;	//TODO: allow n-channel textures
    const uint32_t nBytes(width * height * numlayers * numChannels);
    CreateTexture(width, height, {
        .mipLevels = 1,
        .numLayers = 1,
        .initialData = {{reinterpret_cast<std::byte*>(bytes), nBytes}}
     });
    freer();
}

Texture::Texture(const std::string& name){
    Debug::Assert(IsRasterImage(name), "This texture constructor only allows raster image formats");
    
	//read from resource
	
    RavEngine::Vector<std::byte> data;
	GetApp()->GetResources().FileContentsAt(("/textures/" + name).c_str(),data);
    
    
    MemoryStream stream(data);

    // what kind of texture is this?
    Array<std::byte,16> headerData;
    stream.read(headerData);
    stream.reset();
    if (std::string_view{ reinterpret_cast<const char*>(headerData.data()),4 } == "DDS ") {
        // this is a DDS
        InitFromDDS(stream);
        return;
    }
    if (std::string_view{ reinterpret_cast<const char*>(headerData.data()),2 } == "v/") {
        InitFromEXR(stream);
        return;
    }
    
    std::function<void()> freer;
    const char* failureReason = nullptr;
	
	int width = 0, height = 0,channels;
	auto compressed_size = sizeof(stbi_uc) * data.size();
	
    unsigned char* bytes = stbi_load_from_memory(reinterpret_cast<uint8_t*>(&data[0]), Debug::AssertSize<int>(compressed_size), &width, &height, &channels, 4);
	if (bytes != nullptr){
        freer = [bytes] { stbi_image_free(bytes); };
        goto load;
	}
    else {
        failureReason = stbi_failure_reason();
    }

    // if we are here then we failed to load the image
    if (bytes == nullptr) {
        Debug::Fatal("Cannot load texture {}: {}", name, failureReason);
    }
	
    load:
	uint16_t numlayers = 1;
    uint16_t numChannels = 4;	//TODO: allow n-channel textures

    const uint32_t nBytes(width * height * numlayers * numChannels);
    CreateTexture(width, height, {
        .mipLevels = 1, 
        .numLayers = numlayers,
        .initialData = {{reinterpret_cast<std::byte*>(bytes), nBytes }},
    });
    freer();
	
}


RGL::Dimension RavEngine::Texture::GetTextureSize() const
{
    return texture->GetSize();
}

void Texture::CreateTexture(int width, int height, const Config& config){

	RGL::TextureFormat format = config.format;
	
	auto device = GetApp()->GetDevice();
    if (config.initialData.data.data() != nullptr){
        texture = device->CreateTextureWithData({
            .usage = {.TransferDestination = true, .Sampled = true, .ColorAttachment = config.enableRenderTarget},
            .aspect = {.HasColor = true},
            .width = uint32_t(width),
            .height = uint32_t(height),
            .mipLevels = config.mipLevels,
            .format = format,
            .debugName = config.debugName
            }, {{config.initialData.data.data(), uint32_t(config.initialData.data.size())}}
        );
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
