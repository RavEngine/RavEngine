#include "RenderEngine.hpp"
#include "App.hpp"
#include <bgfx/bgfx.h>
#include <stb_image.h>
#include <Texture.hpp>
#include "BuiltinMaterials.hpp"

using namespace RavEngine;
using namespace std;

//used to store client data for rml
struct TextureHandleStruct{
	bgfx::TextureHandle th = BGFX_INVALID_HANDLE;
	
	~TextureHandleStruct(){
		bgfx::destroy(th);
	}
};

struct CompiledGeoStruct{
	bgfx::VertexBufferHandle vb = BGFX_INVALID_HANDLE;
	bgfx::IndexBufferHandle ib = BGFX_INVALID_HANDLE;
	Rml::TextureHandle th;
	
	~CompiledGeoStruct(){
		bgfx::destroy(vb);
		bgfx::destroy(ib);
		//do not destroy texture here, RML will tell us when to free that separately
	}
};

/**
 Create a texture for use in RML
 @param width the width of the texture in pixels
 @param height the height of the texture in pixels
 @param data the bytes representing the texture
 @return RML texturehandle which is a pointer to a TextureHandleStruct on the heap
 */
static inline Rml::TextureHandle createTexture(int width, int height, const Rml::byte* data){
	auto format = bgfx::TextureFormat::RGBA8;
	
	bool hasMipMaps = false;
	int numLayers = 1;
	int numChannels = 4;
	
	auto uncompressed_size = width * height * numChannels * numLayers;
	
	int flags = BGFX_TEXTURE_SRGB | BGFX_SAMPLER_POINT;
	
	const bgfx::Memory* textureData = bgfx::copy(data, uncompressed_size);
	auto th = bgfx::createTexture2D(width,height,hasMipMaps,numLayers,format,flags,textureData);
	return reinterpret_cast<Rml::TextureHandle>(new TextureHandleStruct{th});
}

double RenderEngine::GetElapsedTime(){
	return App::currentTime();
}

void RenderEngine::SetMouseCursor(const Rml::String &cursor_name){
	throw runtime_error("Not implemented");
}

void RenderEngine::SetClipboardText(const Rml::String &text){
	throw runtime_error("Not implemented");
}

void RenderEngine::GetClipboardText(Rml::String &text){
	throw runtime_error("Not implemented");
}

/// Called by RmlUi when it wants to render geometry that it does not wish to optimise.
void RenderEngine::RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture, const Rml::Vector2f& translation) {
	
	//create vertex and index buffers
	bgfx::VertexBufferHandle vbuf = bgfx::createVertexBuffer(bgfx::copy(vertices, num_vertices * sizeof(vertices[0])), RmlLayout);
	bgfx::IndexBufferHandle ibuf = bgfx::createIndexBuffer(bgfx::copy(indices, num_indices * sizeof(indices[0])));
		
	//create the texture
	if (texture){
		//TODO: set the texture to the handle
	}
	else{
		//TODO: pass the default texture
	}
	
	auto drawmat = glm::translate(currentMatrix, vector3(translation.x,translation.y,Views::FinalBlit));
	guiMaterial->Draw(vbuf, ibuf, drawmat, 0);
	
	//destroy buffers
	bgfx::destroy(vbuf);
	bgfx::destroy(ibuf);

}

/// Called by RmlUi when it wants to compile geometry it believes will be static for the forseeable future.
Rml::CompiledGeometryHandle RenderEngine::CompileGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture){
	
	//create the vertex and index buffers
	bgfx::VertexBufferHandle vbuf = bgfx::createVertexBuffer(bgfx::copy(vertices, num_vertices * sizeof(vertices[0])), RmlLayout);
	bgfx::IndexBufferHandle ibuf = bgfx::createIndexBuffer(bgfx::copy(indices, num_indices * sizeof(indices[0])));
	
	CompiledGeoStruct* cgs = new CompiledGeoStruct{vbuf,ibuf,texture};
	return reinterpret_cast<Rml::CompiledGeometryHandle>(cgs);
	
}
/// Called by RmlUi when it wants to render application-compiled geometry.
void RenderEngine::RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry, const Rml::Vector2f& translation){
	CompiledGeoStruct* cgs = reinterpret_cast<CompiledGeoStruct*>(geometry);
	
	//TODO: determine if need to pass default texture or use texture inside struct
	if(cgs->th){
		//TODO: set texture to handle
	}
	else{
		//TODO: set texture to default texture
	}
	
	auto drawmat = glm::translate(currentMatrix, vector3(translation.x,translation.y,Views::FinalBlit));
	guiMaterial->Draw(cgs->vb, cgs->ib, drawmat, Views::FinalBlit);
	
	//don't delete here, RML will tell us when to delete cgs
}
/// Called by RmlUi when it wants to release application-compiled geometry.
void RenderEngine::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry) {
	CompiledGeoStruct* cgs = reinterpret_cast<CompiledGeoStruct*>(geometry);
	delete cgs; 	//destructor calls bgfx::destroy on appropriate members
}

/// Called by RmlUi when it wants to enable or disable scissoring to clip content.
void RenderEngine::EnableScissorRegion(bool enable) {
	//TODO: control scissor (set it back after GUI is done rendering!)
}
/// Called by RmlUi when it wants to change the scissor region.
void RenderEngine::SetScissorRegion(int x, int y, int width, int height) {
	//TODO: control scissor (set it back after GUI is done rendering!)
}

/// Called by RmlUi when a texture is required by the library.
bool RenderEngine::LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source) {
	
	//pull texture out of vfs into byte array, then call createTexture
	auto data = App::Resources->FileContentsAt(("/textures/" + source).c_str());
	
	int width, height,channels;
	stbi_uc const* datastr = reinterpret_cast<const unsigned char* const>(data.c_str());
	auto compressed_size = sizeof(stbi_uc) * data.size();
	
	unsigned char* bytes = stbi_load_from_memory(datastr, compressed_size, &width, &height, &channels, 4);
	if (bytes == nullptr){
		throw(runtime_error(stbi_failure_reason()));
	}
	
	texture_handle = createTexture(texture_dimensions.x, texture_dimensions.y, bytes);
	
	return true;
}
/// Called by RmlUi when a texture is required to be built from an internally-generated sequence of pixels.
bool RenderEngine::GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions) {
	
	texture_handle = createTexture(source_dimensions.x, source_dimensions.y, source);
	
	return true;
}
/// Called by RmlUi when a loaded texture is no longer required.
void RenderEngine::ReleaseTexture(Rml::TextureHandle texture_handle) {
	TextureHandleStruct* ths = reinterpret_cast<TextureHandleStruct*>(texture_handle);
	delete ths;	//destructor calls bgfx::destroy
}

/// Called by RmlUi when it wants to set the current transform matrix to a new matrix.
void RenderEngine::SetTransform(const Rml::Matrix4f* transform){
	
	auto data = transform->data();
	currentMatrix = glm::make_mat4(data);
}
