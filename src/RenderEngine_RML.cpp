#if !RVE_SERVER
#include "RenderEngine.hpp"
#include "App.hpp"
#include <RGL/RGL.hpp>
#include <stb_image.h>
#include <Texture.hpp>
#include "BuiltinMaterials.hpp"
#include "Common3D.hpp"
#include "Debug.hpp"
#include <SDL_clipboard.h>
#include <RGL/Texture.hpp>
#include <RGL/CommandBuffer.hpp>
#include "VirtualFileSystem.hpp"
#include <glm/gtc/type_ptr.hpp>

using namespace RavEngine;
using namespace std;

//used to store client data for rml
struct TextureHandleStruct{
	RGLTexturePtr th;
	void Destroy(RenderEngine* renderer) {
		renderer->gcTextures.enqueue(th);
	}
};

struct CompiledGeoStruct{
	RGLBufferPtr vb, ib;
	Rml::TextureHandle th;
	const int nindices = 0;

	void Destroy(RenderEngine* renderer) {
		renderer->gcBuffers.enqueue(vb);
		renderer->gcBuffers.enqueue(ib);
	}
	
	~CompiledGeoStruct(){
		//do not destroy texture here, RML will tell us when to free that separately
	}
};

matrix4 RenderEngine::make_gui_matrix(Rml::Vector2f translation){
	matrix4 mat(1);	//start with identity
    dim_t<int> size = { static_cast<int>(currentRenderSize.width), static_cast<int>(currentRenderSize.height) };
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
static inline Rml::TextureHandle createTexture(uint32_t width, uint32_t height, const Rml::byte* data, RGLDevicePtr device){
	int numLayers = 1;
	int numChannels = 4;

	auto uncompressed_size = width * height * numChannels * numLayers;
	
	auto th = device->CreateTextureWithData(
		{
			.usage = {.TransferDestination = true, .Sampled = true},
			.aspect {.HasColor = true},
			.width = width,
			.height = height,
			.format = RGL::TextureFormat::RGBA8_Unorm
		}, 
		{ data, uncompressed_size }
	);

	return reinterpret_cast<Rml::TextureHandle>(new TextureHandleStruct{th});

}

double RenderEngine::GetElapsedTime(){
	return GetApp()->GetCurrentTime();
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

	auto vbuf = device->CreateBuffer({
		 uint32_t(num_vertices),
		{.VertexBuffer = true},
		sizeof(Rml::Vertex),
		RGL::BufferAccess::Private
		});

	auto ibuf = device->CreateBuffer({
		 uint32_t(num_indices),
		{.IndexBuffer = true},
		sizeof(int),
		RGL::BufferAccess::Private
		});

	vbuf->SetBufferData({ vertices, uint32_t(num_vertices * sizeof(Rml::Vertex))});
	ibuf->SetBufferData({ indices, uint32_t(num_indices * sizeof(int)) });

	RGLTexturePtr tx;
	if (texture) {
		auto btexture = reinterpret_cast<TextureHandleStruct*>(texture);
		tx = btexture->th;
	}
	else {
		tx = Texture::Manager::defaultTexture->GetRHITexturePointer();
	}
	auto drawmat = make_gui_matrix(translation);

	mainCommandBuffer->BindRenderPipeline(guiRenderPipeline);
	if (RMLScissor.enabled) {
		mainCommandBuffer->SetScissor({ RMLScissor.x, RMLScissor.y, RMLScissor.width, RMLScissor.height });
	}

	mainCommandBuffer->SetVertexBuffer(vbuf);
	mainCommandBuffer->SetIndexBuffer(ibuf);
	mainCommandBuffer->SetVertexBytes(drawmat, 0);
	mainCommandBuffer->SetFragmentSampler(textureSampler, 0);
	mainCommandBuffer->SetFragmentTexture(tx->GetDefaultView(), 1);
	mainCommandBuffer->DrawIndexed(num_indices);

	// trash buffers
	gcBuffers.enqueue(vbuf);
	gcBuffers.enqueue(ibuf);
}

/// Called by RmlUi when it wants to compile geometry it believes will be static for the forseeable future.
Rml::CompiledGeometryHandle RenderEngine::CompileGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture){
	auto vbuf = device->CreateBuffer({
		 uint32_t(num_vertices),
		{.VertexBuffer = true},
		sizeof(Rml::Vertex),
		RGL::BufferAccess::Private
	});

	auto ibuf = device->CreateBuffer({
		 uint32_t(num_indices),
		{.IndexBuffer = true},
		sizeof(int),
		RGL::BufferAccess::Private
	});

	vbuf->SetBufferData({vertices, uint32_t(num_vertices * sizeof(Rml::Vertex))});
	ibuf->SetBufferData({indices, uint32_t(num_indices * sizeof(int))});

	CompiledGeoStruct* cgs = new CompiledGeoStruct{ vbuf,ibuf, texture, num_indices };
	return reinterpret_cast<Rml::CompiledGeometryHandle>(cgs);
}
/// Called by RmlUi when it wants to render application-compiled geometry.
void RenderEngine::RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry, const Rml::Vector2f& translation){
	CompiledGeoStruct* cgs = reinterpret_cast<CompiledGeoStruct*>(geometry);

	RGLTexturePtr tx;
	if (cgs->th) {
		auto btexture = reinterpret_cast<TextureHandleStruct*>(cgs->th);
		tx = btexture->th;
	}
	else {
		tx = Texture::Manager::defaultTexture->GetRHITexturePointer();
	}
	mainCommandBuffer->BindRenderPipeline(guiRenderPipeline);
	if (RMLScissor.enabled) {
		mainCommandBuffer->SetScissor({ RMLScissor.x, RMLScissor.y, RMLScissor.width, RMLScissor.height });
	}
	auto drawmat = make_gui_matrix(translation);

	mainCommandBuffer->SetVertexBuffer(cgs->vb);
	mainCommandBuffer->SetIndexBuffer(cgs->ib);
	mainCommandBuffer->SetVertexBytes(drawmat, 0);
	mainCommandBuffer->SetFragmentSampler(textureSampler, 0);
	mainCommandBuffer->SetFragmentTexture(tx->GetDefaultView(), 1);
	mainCommandBuffer->DrawIndexed(cgs->nindices);

	//don't delete here, RML will tell us when to delete cgs
}
/// Called by RmlUi when it wants to release application-compiled geometry.
void RenderEngine::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry) {
	CompiledGeoStruct* cgs = reinterpret_cast<CompiledGeoStruct*>(geometry);
	cgs->Destroy(this);	// enqueue buffers for deletion on the next frame
	delete cgs; 	//destructor decrements refcounts as needed
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
    RavEngine::Vector<uint8_t> data;
	GetApp()->GetResources().FileContentsAt((source).c_str(),data);
	
	int width, height,channels;
	auto compressed_size = sizeof(stbi_uc) * data.size();
	
	unsigned char* bytes = stbi_load_from_memory(&data[0], Debug::AssertSize<int>(compressed_size), &width, &height, &channels, 4);
	if (bytes == nullptr){
		Debug::Fatal("Cannot open image: {}",stbi_failure_reason());
	}
	texture_dimensions.x = width;
	texture_dimensions.y = height;
	
	texture_handle = createTexture(texture_dimensions.x, texture_dimensions.y, bytes, device);
	
	return true;
}
/// Called by RmlUi when a texture is required to be built from an internally-generated sequence of pixels.
bool RenderEngine::GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions) {
	
	texture_handle = createTexture(source_dimensions.x, source_dimensions.y, source, device);
	
	return true;
}
/// Called by RmlUi when a loaded texture is no longer required.
void RenderEngine::ReleaseTexture(Rml::TextureHandle texture_handle) {
	TextureHandleStruct* ths = reinterpret_cast<TextureHandleStruct*>(texture_handle);
	ths->Destroy(this);	// enqueue texture for deletion
	delete ths;
}

/// Called by RmlUi when it wants to set the current transform matrix to a new matrix.
void RenderEngine::SetTransform(const Rml::Matrix4f* transform){
	
	auto data = transform->data();
	currentGUIMatrix = glm::make_mat4(data);
	Debug::Fatal("Local transformations not supported yet");
}
#endif
