
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
SpinLock Material::Manager::mtx;

// mapping names to types
const unordered_map<string, ShaderStage> stagemap{
	{"vertex",ShaderStage::Vertex},
	{"fragment",ShaderStage::Fragment},
	{"geometry",ShaderStage::Geometry},
	{"tesseval",ShaderStage::TessEval},
	{"tesscontrol",ShaderStage::TessControl},
	{"compute",ShaderStage::Compute},
	
};

void RavEngine::Material::SetTransformMatrix(const matrix4& mat)
{
    transformMatrix = mat;
}

bgfx::ShaderHandle loadShader(const string& data){
	const bgfx::Memory* mem = bgfx::copy(data.c_str(), data.size());
	return bgfx::createShader(mem);
}

void Material::Draw(const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer)
{
    //calculate wvp matrix
    const auto& view = Material::Manager::GetCurrentViewMatrix();
    const auto& projection = Material::Manager::GetCurrentProjectionMatrix();
    //auto wvp = projection * view * transformMatrix; //transformMatrix * view * projection;

	//copy into backend matrix
	float viewmat[16];
	float projmat[16];
	float transmat[16];
    const decimalType* vS = (const decimalType*)glm::value_ptr(view);
	const decimalType* pS = (const decimalType*)glm::value_ptr(projection);
	const decimalType* tS = (const decimalType*)glm::value_ptr(transformMatrix);
    for (int i = 0; i < 16; ++i) {
		viewmat[i] = vS[i];
		projmat[i] = pS[i];
		transmat[i] = tS[i];
    }

	bgfx::setViewTransform(0, viewmat, projmat);
	bgfx::setTransform(transmat);
	
	//set vertex and index buffer
	bgfx::setVertexBuffer(0, vertexBuffer);
	bgfx::setIndexBuffer(indexBuffer);

	bgfx::submit(0, program);
}

/**
Create a material given a shader. Also registers it in the material manager
@param shader the path to the shader
*/
Material::Material(const std::string& name) : name(name) {	
	//get all shader files for this programs
	string dir = "shaders/" + name + ".tar";

	auto data = App::Resources->FileContentsAt(dir);

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
	mtx.lock();
	result = materials.find(t) != materials.end();
	mtx.unlock();
	return result;
}
