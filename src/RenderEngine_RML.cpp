#include "RenderEngine.hpp"
#include "App.hpp"
#include <bgfx/bgfx.h>
#include <stb_image.h>
#include <Texture.hpp>
#include "BuiltinMaterials.hpp"
#include "Common3D.hpp"
#include "Debug.hpp"
#include <SDL_clipboard.h>

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

static inline void RML2BGFX(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, const bgfx::VertexLayout& RmlLayout, bgfx::VertexBufferHandle& out_vb, bgfx::IndexBufferHandle& out_ib){
	
	
	//create vertex and index buffers
	{
		struct SVec2{
			float x, y, u, v;
			color_t color;
		};
		stackarray(convertedv, SVec2, num_vertices);
		
		for(int i = 0; i < num_vertices; i++){
			Rml::Vertex v = vertices[i];
			convertedv[i].x = v.position.x;
			convertedv[i].y = v.position.y;
			convertedv[i].u = v.tex_coord.x;
			convertedv[i].v = v.tex_coord.y;
			
			color_t color = (v.colour.alpha << 24) + (v.colour.blue << 16) + (v.colour.green << 8) + v.colour.red;
			
			convertedv[i].color = color;
		}
		out_vb = bgfx::createVertexBuffer(bgfx::copy(convertedv, num_vertices * sizeof(convertedv[0])), RmlLayout);
	}
	{
		stackarray(convertedi, uint16_t, num_indices);
		
		for(int i = 0; i < num_indices; i++){
			convertedi[i] = indices[i];
		}
		out_ib = bgfx::createIndexBuffer(bgfx::copy(convertedi, num_indices * sizeof(convertedi[0])));
	}
	
}

static inline matrix4 make_matrix(Rml::Vector2f translation){
	matrix4 mat(1);	//start with identity
	auto size = App::Renderer->GetBufferSize();
	mat = glm::scale(mat, vector3(1,-1,1));	//flip
	mat = glm::scale(mat, vector3(1.0/(size.width/2.0),1.0/(size.height/2.0),1));	//scale into view space
	mat = glm::translate(mat, vector3(-size.width/2.0,-size.height/2.0,0));			//translate to origin-center
	mat = glm::translate(mat, vector3(translation.x,translation.y,0)); 				//pixel-space offset
	return mat;
}

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
	Debug::Fatal("Not implemented");
}

void RenderEngine::SetClipboardText(const Rml::String &text){
	SDL_SetClipboardText(text.c_str());
}

void RenderEngine::GetClipboardText(Rml::String &text){
	auto data = SDL_GetClipboardText();
	text = data;		//avoid this copy?
	SDL_free(data);
}

/// Called by RmlUi when it wants to render geometry that it does not wish to optimise.
void RenderEngine::RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture, const Rml::Vector2f& translation) {
	
	bgfx::VertexBufferHandle vbuf = BGFX_INVALID_HANDLE;
	bgfx::IndexBufferHandle ibuf = BGFX_INVALID_HANDLE;
	
	RML2BGFX(vertices, num_vertices, indices, num_indices, RmlLayout, vbuf, ibuf);
		
	//create the texture
	bgfx::TextureHandle tx = BGFX_INVALID_HANDLE;
	if (texture){
		auto btexture = reinterpret_cast<TextureHandleStruct*>(texture);
		tx = btexture->th;
	}
	else{
		tx = TextureManager::defaultTexture->get();
	}
	
	guiMaterial->SetTexture(tx);
	
	auto drawmat = make_matrix(translation);
    if (RMLScissor.enabled){
        bgfx::setScissor(RMLScissor.x, RMLScissor.y, RMLScissor.width, RMLScissor.height);
    }
	bgfx::setState( BGFX_STATE_WRITE_RGB | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) );
	guiMaterial->Draw(vbuf, ibuf, drawmat, Views::FinalBlit);
	
	//destroy buffers
	bgfx::destroy(vbuf);
	bgfx::destroy(ibuf);

}

/// Called by RmlUi when it wants to compile geometry it believes will be static for the forseeable future.
Rml::CompiledGeometryHandle RenderEngine::CompileGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture){
	
	//create the vertex and index buffers
	bgfx::VertexBufferHandle vbuf = BGFX_INVALID_HANDLE;
	bgfx::IndexBufferHandle ibuf = BGFX_INVALID_HANDLE;
	RML2BGFX(vertices, num_vertices, indices, num_indices, RmlLayout, vbuf, ibuf);
	
	CompiledGeoStruct* cgs = new CompiledGeoStruct{vbuf,ibuf,texture};
	return reinterpret_cast<Rml::CompiledGeometryHandle>(cgs);
	
}
/// Called by RmlUi when it wants to render application-compiled geometry.
void RenderEngine::RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry, const Rml::Vector2f& translation){
	CompiledGeoStruct* cgs = reinterpret_cast<CompiledGeoStruct*>(geometry);
	
	bgfx::TextureHandle tx = BGFX_INVALID_HANDLE;
	if(cgs->th){
		auto btexture = reinterpret_cast<TextureHandleStruct*>(cgs->th);
		tx = btexture->th;
	}
	else{
		tx = TextureManager::defaultTexture->get();
	}
	
	guiMaterial->SetTexture(tx);

	auto drawmat = make_matrix(translation);
    if (RMLScissor.enabled){
        bgfx::setScissor(RMLScissor.x, RMLScissor.y, RMLScissor.width, RMLScissor.height);
    }
	bgfx::setState( BGFX_STATE_WRITE_RGB | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) );
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
    RMLScissor.enabled = enable;
}
/// Called by RmlUi when it wants to change the scissor region.
void RenderEngine::SetScissorRegion(int x, int y, int width, int height) {
    RMLScissor.x = x;
    RMLScissor.y = y;
    RMLScissor.width = width;
    RMLScissor.height = height;
}

/// Called by RmlUi when a texture is required by the library.
bool RenderEngine::LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source) {
	
	//pull texture out of vfs into byte array, then call createTexture
	std::vector<uint8_t> data;
	App::Resources->FileContentsAt((source).c_str(),data);
	
	int width, height,channels;
	auto compressed_size = sizeof(stbi_uc) * data.size();
	
	unsigned char* bytes = stbi_load_from_memory(&data[0], compressed_size, &width, &height, &channels, 4);
	if (bytes == nullptr){
		Debug::Fatal("Cannot open image: {}",stbi_failure_reason());
	}
	texture_dimensions.x = width;
	texture_dimensions.y = height;
	
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
	Debug::Fatal("Local transformations not supported yet");
}
