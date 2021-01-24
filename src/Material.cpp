
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

Material::Manager::MaterialStore Material::Manager::materials;

// mapping names to types
const phmap::flat_hash_map<string, ShaderStage> stagemap{
	{"vertex",ShaderStage::Vertex},
	{"fragment",ShaderStage::Fragment},
	{"geometry",ShaderStage::Geometry},
	{"tesseval",ShaderStage::TessEval},
	{"tesscontrol",ShaderStage::TessControl},
	{"compute",ShaderStage::Compute},
	
};


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

/**
Create a material given a shader. Also registers it in the material manager
@param shader the path to the shader
*/
Material::Material(const std::string& name) : name(name) {	
	//get all shader files for this programs
	string dir = "/shaders/" + name;

	std::vector<uint8_t> vertex_src, fragment_src;
	App::Resources->FileContentsAt((dir + "/vertex.bin").c_str(),vertex_src);
	App::Resources->FileContentsAt((dir + "/fragment.bin").c_str(),fragment_src);

	//must have a vertex and a fragment shader
	bgfx::ShaderHandle vsh = loadShader(vertex_src);
	bgfx::ShaderHandle fsh = loadShader(fragment_src);
	program = bgfx::createProgram(vsh, fsh, true);
	if (!bgfx::isValid(program)){
		Debug::Fatal("Material is invalid.");
	}
}


bool Material::Manager::HasMaterialByTypeIndex(const ctti_t t){
	bool result = false;
	result = materials.find(t) != materials.end();
	return result;
}
