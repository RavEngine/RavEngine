
#include "Material.hpp"
#include <sstream>
#include <fstream>
#include <RenderEngine.hpp>
#include "mathtypes.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <filesystem>
#include "Common3D.hpp"
#include <bgfx/bgfx.h>
#include "App.hpp"
#include <typeindex>
#include "Debug.hpp"

using namespace std;
using namespace RavEngine;
using namespace std::filesystem;

STATIC(Material::Manager::materials);
STATIC(Material::Manager::mtx);

bgfx::ShaderHandle loadShader(const std::vector<uint8_t>& data){
	const bgfx::Memory* mem = bgfx::copy(&data[0], data.size());
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

	std::vector<uint8_t> vertex_src, fragment_src;
	App::Resources->FileContentsAt(StrFormat("{}/vertex.bin",dir).c_str(),vertex_src);
	App::Resources->FileContentsAt(StrFormat("{}/fragment.bin",dir).c_str(),fragment_src);

	//must have a vertex and a fragment shader
	bgfx::ShaderHandle vsh = loadShader(vertex_src);
	bgfx::ShaderHandle fsh = loadShader(fragment_src);
	program = bgfx::createProgram(vsh, fsh, true);
	if (!bgfx::isValid(program)){
		Debug::Fatal("Material is invalid.");
	}
}

bgfx::ProgramHandle RavEngine::Material::getShaderHandle(const std::string_view& full_path)
{
	vector<uint8_t> shaderdata;
	App::Resources->FileContentsAt(StrFormat("shaders/{}/{}", shader_api(), full_path).c_str(), shaderdata);
	const bgfx::Memory* mem = bgfx::copy(&shaderdata[0], shaderdata.size());
	return bgfx::createProgram(bgfx::createShader(mem), true);	//auto destroys shader when program is destroyed
}
