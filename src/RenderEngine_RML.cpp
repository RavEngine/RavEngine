#if !RVE_SERVER
#include "RenderEngine.hpp"
#include "App.hpp"
#include "Window.hpp"
#include <RGL/RGL.hpp>
#include <stb_image.h>
#include <Texture.hpp>
#include "BuiltinMaterials.hpp"
#include "Common3D.hpp"
#include "Debug.hpp"
#include <SDL3/SDL_clipboard.h>
#include <RGL/Texture.hpp>
#include <RGL/CommandBuffer.hpp>
#include "VirtualFileSystem.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <SDL3/SDL_keyboard.h>

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
	
	const uint32_t nindices = 0;

	void Destroy(RenderEngine* renderer) {
		renderer->gcBuffers.enqueue(vb);
		renderer->gcBuffers.enqueue(ib);
	}
	
};

struct CompiledTextureStruct {
	Rml::TextureHandle th;
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
static inline Rml::TextureHandle createTexture(uint32_t width, uint32_t height, Rml::Span<const Rml::byte> data, RGLDevicePtr device){
		
	auto th = device->CreateTextureWithData(
		{
			.usage = {.TransferDestination = true, .Sampled = true},
			.aspect {.HasColor = true},
			.width = width,
			.height = height,
			.format = RGL::TextureFormat::RGBA8_Unorm
		}, 
		{ {data.data(), data.size()}}
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


/// Called by RmlUi when it wants to compile geometry it believes will be static for the forseeable future.
Rml::CompiledGeometryHandle RenderEngine::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices){
	const auto num_vertices = vertices.size();
	const auto num_indices = indices.size();
	auto vbuf = device->CreateBuffer({
		 uint32_t(num_vertices),
		{.VertexBuffer = true},
		sizeof(Rml::Vertex),
		RGL::BufferAccess::Private,
		{.debugName = "RML Compiled Vertex Bufer"}
	});

	auto ibuf = device->CreateBuffer({
		 uint32_t(num_indices),
		{.IndexBuffer = true},
		sizeof(int),
		RGL::BufferAccess::Private,
		{.debugName = "RML Compiled Index Bufer"}
	});

	// first half is for vertex data, second half is for index data
	const auto vertSize = num_vertices * sizeof(Rml::Vertex);
	const auto indsize = num_indices * sizeof(int);

	if (RGL::CurrentAPI() == RGL::API::Direct3D12) {
		auto vbufStaging = WriteTransient({ vertices.data(), vertSize});
		auto ibufStaging = WriteTransient({ indices.data(), indsize});

		mainCommandBuffer->CopyBufferToBuffer(
			{
				.buffer = transientBuffer,
				.offset = vbufStaging
			},
		{
			.buffer = vbuf,
			.offset = 0
		}, vertSize);

		mainCommandBuffer->CopyBufferToBuffer(
			{
				.buffer = transientBuffer,
				.offset = ibufStaging
			},
		{
			.buffer = ibuf,
			.offset = 0
		},
			indsize
		);
	}
	else {
		// vulkan requires us to do something inefficient here
		vbuf->SetBufferData({ vertices.data(), uint32_t(num_vertices * sizeof(Rml::Vertex))});
		ibuf->SetBufferData({ indices.data(), uint32_t(num_indices * sizeof(int))});
	}

	CompiledGeoStruct* cgs = new CompiledGeoStruct{ vbuf,ibuf, uint32_t(num_indices) };
	return reinterpret_cast<Rml::CompiledGeometryHandle>(cgs);
}
/// Called by RmlUi when it wants to render application-compiled geometry.
void RenderEngine::RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture){
	CompiledGeoStruct* cgs = reinterpret_cast<CompiledGeoStruct*>(geometry);

	RGLTexturePtr tx;
	if (texture) {
		auto btexture = reinterpret_cast<TextureHandleStruct*>(texture);
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
	drawmat = drawmat * currentGUIMatrix;		// apply requested transformation

	mainCommandBuffer->SetVertexBuffer(cgs->vb);
	mainCommandBuffer->SetIndexBuffer(cgs->ib);
	mainCommandBuffer->SetVertexBytes(drawmat, 0);
	mainCommandBuffer->SetFragmentSampler(textureSampler, 0);
	mainCommandBuffer->SetFragmentTexture(tx->GetDefaultView(), 1);
	mainCommandBuffer->DrawIndexed(cgs->nindices);

	//don't delete here, RML will tell us when to delete cgs
}
/// Called by RmlUi when it wants to release application-compiled geometry.
void RenderEngine::ReleaseGeometry(Rml::CompiledGeometryHandle geometry) {
	CompiledGeoStruct* cgs = reinterpret_cast<CompiledGeoStruct*>(geometry);
	cgs->Destroy(this);	// enqueue buffers for deletion on the next frame
	delete cgs; 	//destructor decrements refcounts as needed
}

/// Called by RmlUi when it wants to enable or disable scissoring to clip content.
void RenderEngine::EnableScissorRegion(bool enable) {
    RMLScissor.enabled = enable;
}
/// Called by RmlUi when it wants to change the scissor region.
void RenderEngine::SetScissorRegion(Rml::Rectanglei region) {
	const auto xy = region.TopLeft();
    RMLScissor.x = xy.x;
    RMLScissor.y = xy.y;
    RMLScissor.width = region.Width();
    RMLScissor.height = region.Height();
}

/// Called by RmlUi when a texture is required by the library.
Rml::TextureHandle RenderEngine::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) {
	
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

	constexpr int numLayers = 1;
	constexpr int numChannels = 4;
	auto uncompressed_size = width * height * numChannels * numLayers;
	
	return createTexture(texture_dimensions.x, texture_dimensions.y, {bytes, uint32_t(uncompressed_size) }, device);
}
/// Called by RmlUi when a texture is required to be built from an internally-generated sequence of pixels.
Rml::TextureHandle RenderEngine::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) {
	
	return createTexture(source_dimensions.x, source_dimensions.y, source, device);
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
}
bool RavEngine::RenderEngine::LogMessage(Rml::Log::Type type, const Rml::String& message)
{
	switch (type) {
	case Rml::Log::Type::LT_ERROR:
	case Rml::Log::Type::LT_ASSERT:
		Debug::Fatal(message);
		break;
	default:
		Debug::Log(message);
		break;
	}

	return true;
}

void RenderEngine::ActivateKeyboard(Rml::Vector2f caret_position, float line_height){
    const SDL_Rect rect{int(caret_position.x), int(caret_position.y), 1, int(line_height)};
    const auto window = GetApp()->GetMainWindow()->window;
    
    SDL_SetTextInputArea(window, &rect, 0);
    SDL_StartTextInput(window);
}

void RenderEngine::DeactivateKeyboard(){
    const auto window = GetApp()->GetMainWindow()->window;
    SDL_StopTextInput(window);
}

#endif
