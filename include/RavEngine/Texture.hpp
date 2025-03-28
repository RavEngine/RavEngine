#if !RVE_SERVER
#pragma once
#include "Ref.hpp"
#include "Manager.hpp"
#include "Filesystem.hpp"
#include <RGL/Types.hpp>
#include <RGL/TextureFormat.hpp>
#include <RGL/Texture.hpp>
#include "RenderTargetCollection.hpp"
#include <span>
#include <cstddef>

namespace RavEngine{

struct IStream;

struct Texture {
	struct Config {
		uint8_t mipLevels = 1;
		int numLayers = 1;
		bool enableRenderTarget = false;
		RGL::TextureUploadData initialData;
		RGL::TextureFormat format = RGL::TextureFormat::RGBA8_Unorm;
		std::string_view debugName;
	};

	/**
	 Create a texture given a file
	 @param filename name of the texture
	 */
	Texture(const std::string& filename);
	Texture(const char* filename) : Texture(std::string(filename)) {}
    
    Texture(const std::string& filename, uint16_t width, uint16_t height);

	Texture(const Filesystem::Path& pathOnDisk);
    Texture(RGLTexturePtr tx) : texture(tx){}

	/**
	 Create a texture from data
	 @param width width of the texture
	 @param height height of the texture
	 @param hasMipMaps does the texture contain mip maps
	 @param numLayers the number of layers in the texture (NOT channels!)
	 @param data pointer to the image data. Must be a 4-channel image.
	 @param flags optional creation flags
	 */
	Texture(int width, int height, const Config& config) : Texture() {
		CreateTexture(width, height, config);
	}
	
	virtual ~Texture();
	    
    /**
     Use the manager to avoid loading duplicate textures
     */
    struct Manager : public GenericWeakReadThroughCache<std::string, RavEngine::Texture>{
		static Ref<RavEngine::Texture> defaultTexture, defaultNormalTexture, zeroTexture;
	};

	auto GetRHITexturePointer() const {
		return texture;
	}
	
	/**
	@return size of the texture in pixels
	*/
	RGL::Dimension GetTextureSize() const;

protected:

	RGLTexturePtr texture;

	Texture(){}
	
	void CreateTexture(int width, int height, const Config& config);

	void InitFromDDS(IStream&);
	void InitFromEXR(IStream&);
};


struct RenderTexture {

	RenderTexture(int width, int height);
    
    Ref<Texture> GetTexture();
    const auto& GetCollection() const{
        return collection;
    }
private:
    RenderTargetCollection collection;
    Ref<Texture> finalFB;
};

struct CubemapTexture {
	struct Config {
		std::string_view debugName;
		RGL::TextureFormat format = RGL::TextureFormat::RGBA8_Unorm;
		uint32_t numMips = 1;
		bool enableRenderTarget = false;
#if !_MSC_VER
        Config(): enableRenderTarget(false), format(RGL::TextureFormat::RGBA8_Unorm),numMips(1){} // workaround for clang/gcc bug
#endif
	};

	CubemapTexture(int size, const Config& config = {});

	RGL::TextureView GetView() const;

	RGL::Dimension GetTextureSize() const;

	auto GetRHITexturePointer() const {
		return cubemap;
	}

private:
	RGLTexturePtr cubemap;
};

}
#endif
