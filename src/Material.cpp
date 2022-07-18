
#include "Material.hpp"
#include <sstream>
#include <fstream>
#include <RenderEngine.hpp>
#include "mathtypes.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Filesystem.hpp"
#include "Common3D.hpp"
#include <bgfx/bgfx.h>
#include "App.hpp"
#include <typeindex>
#include "Debug.hpp"

using namespace std;
using namespace RavEngine;

bgfx::ShaderHandle loadShader(const  RavEngine::Vector<uint8_t>& data){
    Debug::AssertSize<uint32_t>(data.size());
	const bgfx::Memory* mem = bgfx::copy(&data[0], static_cast<uint32_t>(data.size()));
	return bgfx::createShader(mem);
}

void Material::Draw(const bgfx::VertexBufferHandle& vertexBuffer, const bgfx::IndexBufferHandle& indexBuffer, int view)
{	
	//set vertex and index buffer
	bgfx::setVertexBuffer(0, vertexBuffer);
	bgfx::setIndexBuffer(indexBuffer);

	bgfx::submit(view, program);
}

// return the folder name the shaders are stored in
static inline std::string_view shader_api() {
	switch (bgfx::getRendererType()) {
	case bgfx::RendererType::Direct3D11:
	case bgfx::RendererType::Direct3D12:
		return "dx";
	case bgfx::RendererType::Metal:			
		return "mtl";
	case bgfx::RendererType::Vulkan:		
		return "vk";
	case bgfx::RendererType::OpenGL:
		return "gl";
	default:
		Debug::Fatal("Current backend is not supported.");
		return "";
	}
}

/**
Create a material given a shader. Also registers it in the material manager
@param shader the path to the shader
*/
Material::Material(const std::string& name) : name(name) {	
	//get all shader files for this programs
	auto dir = StrFormat("shaders/{}/{}", shader_api(), name);

    RavEngine::Vector<uint8_t> vertex_src, fragment_src;
	GetApp()->GetResources().FileContentsAt(StrFormat("{}/vertex.bin",dir).c_str(),vertex_src);
	GetApp()->GetResources().FileContentsAt(StrFormat("{}/fragment.bin",dir).c_str(),fragment_src);

	//must have a vertex and a fragment shader
	bgfx::ShaderHandle vsh = loadShader(vertex_src);
	bgfx::ShaderHandle fsh = loadShader(fragment_src);
	program = bgfx::createProgram(vsh, fsh, true);
	if (!bgfx::isValid(program)){
		Debug::Fatal("Material is invalid.");
	}
	bgfx::setName(vsh, fmt::format("Material VS {}",name).c_str());
	bgfx::setName(fsh, fmt::format("Material FS {}",name).c_str());
}

bgfx::ShaderHandle Material::loadShaderHandle(const std::string_view& full_path){
    RavEngine::Vector<uint8_t> shaderdata;
	GetApp()->GetResources().FileContentsAt(StrFormat("shaders/{}/{}", shader_api(), full_path).c_str(), shaderdata);
	assert(shaderdata.size() < numeric_limits<uint32_t>::max());
    const bgfx::Memory* mem = bgfx::copy(&shaderdata[0], static_cast<uint32_t>(shaderdata.size()));
    return bgfx::createShader(mem);
}

bgfx::ProgramHandle RavEngine::Material::loadComputeProgram(const std::string_view& full_path)
{
    RavEngine::Vector<uint8_t> shaderdata;
	GetApp()->GetResources().FileContentsAt(StrFormat("shaders/{}/{}", shader_api(), full_path).c_str(), shaderdata);
    Debug::AssertSize<uint32_t>(shaderdata.size());
	const bgfx::Memory* mem = bgfx::copy(&shaderdata[0], static_cast<uint32_t>(shaderdata.size()));
	return bgfx::createProgram(bgfx::createShader(mem), true);	//auto destroys shader when program is destroyed
}

bgfx::ProgramHandle Material::loadShaderProgram(const std::string_view& name_path){
    auto vert = Material::loadShaderHandle(StrFormat("{}/vertex.bin",name_path));
    auto frag = Material::loadShaderHandle(StrFormat("{}/fragment.bin",name_path));
    return bgfx::createProgram(vert, frag);
}
