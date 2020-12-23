#include "RenderEngine.hpp"
#include "App.hpp"

using namespace RavEngine;
using namespace std;

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
	
}

/// Called by RmlUi when it wants to compile geometry it believes will be static for the forseeable future.
Rml::CompiledGeometryHandle RenderEngine::CompileGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture){
	
}
/// Called by RmlUi when it wants to render application-compiled geometry.
void RenderEngine::RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry, const Rml::Vector2f& translation){
	
}
/// Called by RmlUi when it wants to release application-compiled geometry.
void RenderEngine::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry) {
	
}

/// Called by RmlUi when it wants to enable or disable scissoring to clip content.
void RenderEngine::EnableScissorRegion(bool enable) {
	
}
/// Called by RmlUi when it wants to change the scissor region.
void RenderEngine::SetScissorRegion(int x, int y, int width, int height) {
	
}

/// Called by RmlUi when a texture is required by the library.
bool RenderEngine::LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source) {
	
}
/// Called by RmlUi when a texture is required to be built from an internally-generated sequence of pixels.
bool RenderEngine::GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions) {
	
}
/// Called by RmlUi when a loaded texture is no longer required.
void RenderEngine::ReleaseTexture(Rml::TextureHandle texture_handle) {
	
}

/// Called by RmlUi when it wants to set the current transform matrix to a new matrix.
void RenderEngine::SetTransform(const Rml::Matrix4f* transform){
	
}
