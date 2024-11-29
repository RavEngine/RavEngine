#if !RVE_SERVER
#pragma once
#include "Ref.hpp"
#include "Manager.hpp"
#include "Filesystem.hpp"
#include <RGL/Types.hpp>
#include <RGL/TextureFormat.hpp>
#include "RenderTargetCollection.hpp"
#include <span>
#include <cstddef>

namespace RavEngine{

struct IStream;

class Texture {
public:
	/**
	 Create a texture given a file
	 @param filename name of the texture
	 */
	Texture(const std::string& filename);
	Texture(const char* filename) : Texture(std::string(filename)) {}
    
    Texture(const std::string& filename, uint16_t width, uint16_t height);

	Texture(const Filesystem::Path& pathOnDisk);
    Texture(RGLTexturePtr tx) : texture(tx){}
	
	virtual ~Texture();
	    
    /**
     Use the manager to avoid loading duplicate textures
     Works with Runtime textures as well, using the construction arguments to differentiate textures
     */
    struct Manager : public GenericWeakReadThroughCache<std::string,Texture>{
		static Ref<class RuntimeTexture> defaultTexture, defaultNormalTexture, zeroTexture;
	};

	auto GetRHITexturePointer() const {
		return texture;
	}

	struct Config {
		uint8_t mipLevels = 1;
		int numLayers = 1;
		bool enableRenderTarget = false;
		const std::span<std::byte> initialData;
		RGL::TextureFormat format = RGL::TextureFormat::RGBA8_Unorm;
		std::string_view debugName;
	};
	
protected:

	RGLTexturePtr texture;

	Texture(){}
	
	void CreateTexture(int width, int height, const Config& config);

	void InitFromDDS(IStream&);
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
	RuntimeTexture(int width, int height, const Config& config) : Texture(){
		CreateTexture(width, height, config);
	}
};

class RenderTexture {
public:
    RenderTexture(int width, int height);
    
    Ref<RuntimeTexture> GetTexture();
    const auto& GetCollection() const{
        return collection;
    }
private:
    RenderTargetCollection collection;
    Ref<RuntimeTexture> finalFB;
};

}
#endif
