
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
#include <ravtar/tarball.hpp>
#include <typeindex>

using namespace std;
using namespace RavEngine;
using namespace std::filesystem;

Material::Manager::MaterialStore Material::Manager::materials;
matrix4 Material::Manager::projectionMatrix;
matrix4 Material::Manager::viewMatrix;
matrix4 Material::Manager::transformMatrix = matrix4(1);

// mapping names to types
const phmap::flat_hash_map<string, ShaderStage> stagemap{
	{"vertex",ShaderStage::Vertex},
	{"fragment",ShaderStage::Fragment},
	{"geometry",ShaderStage::Geometry},
	{"tesseval",ShaderStage::TessEval},
	{"tesscontrol",ShaderStage::TessControl},
	{"compute",ShaderStage::Compute},
	
};


bgfx::ShaderHandle loadShader(const string& data){
	const bgfx::Memory* mem = bgfx::copy(data.c_str(), data.size());
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
	string dir = "/shaders/" + name + ".tar";

	auto data = App::Resources->FileContentsAt(dir.c_str());

	std::istringstream istr(data);
	Tar::TarReader reader(istr);

	//get the shader code
	auto vertex_src = reader.GetFile("vertex.bin");
	auto fragment_src = reader.GetFile("fragment.bin");

	//must have a vertex and a fragment shader
	bgfx::ShaderHandle vsh = loadShader(vertex_src);
	bgfx::ShaderHandle fsh = loadShader(fragment_src);
	program = bgfx::createProgram(vsh, fsh, true);
	if (!bgfx::isValid(program)){
		throw runtime_error("Material is invalid.");
	}
}


bool Material::Manager::HasMaterialByTypeIndex(const std::type_index& t){
	bool result = false;
	result = materials.find(t) != materials.end();
	return result;
}
