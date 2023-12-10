#if !RVE_SERVER
#pragma once
#include "Ref.hpp"
#include "Manager.hpp"
#include "Filesystem.hpp"
#include <RGL/Types.hpp>
#include <RGL/TextureFormat.hpp>

namespace RavEngine{

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
	
	virtual ~Texture();
	    
    /**
     Use the manager to avoid loading duplicate textures
     Works with Runtime textures as well, using the construction arguments to differentiate textures
     */
    struct Manager : public GenericWeakReadThroughCache<std::string,Texture>{
		static Ref<class RuntimeTexture> defaultTexture, defaultNormalTexture;
	};

	auto GetRHITexturePointer() const {
		return texture;
	}

	struct Config {
		uint8_t mipLevels = 1;
		int numLayers = 1;
		bool enableRenderTarget = false;
		const uint8_t* initialData = nullptr;
		RGL::TextureFormat format = RGL::TextureFormat::RGBA8_Unorm;
		const char* const debugName = nullptr;
	};
	
protected:

	RGLTexturePtr texture;

	Texture(){}
	
	void CreateTexture(int width, int height, const Config& config);
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

}
#endif
