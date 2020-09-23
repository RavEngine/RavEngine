
#include "Material.hpp"
#include <sstream>
#include <fstream>
#include <RenderEngine.hpp>
#include "mathtypes.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <filesystem>
#include "Common3D.hpp"
#include <bgfx/bgfx.h>
#include <cmrc/cmrc.hpp>
#include <tarball.hpp>

using namespace std;
using namespace RavEngine;
using namespace std::filesystem;
using namespace Tar;

CMRC_DECLARE(RavEngine_RSC);

static const cmrc::embedded_filesystem shaderfiles = cmrc::RavEngine_RSC::get_filesystem();

MaterialManager::MaterialStore MaterialManager::materials;
matrix4 MaterialManager::projectionMatrix;
matrix4 MaterialManager::viewMatrix;
mutex MaterialManager::mtx;

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

bgfx::ShaderHandle loadShader(const std::string& data){

	const bgfx::Memory* mem = bgfx::copy(data.c_str(), data.size());
	
	return bgfx::createShader(mem);
}

void Material::Draw(const bgfx::VertexBufferHandle& vertexBuffer, const bgfx::IndexBufferHandle& indexBuffer)
{
    //calculate wvp matrix
    const auto& view = MaterialManager::GetCurrentViewMatrix();
    const auto& projection = MaterialManager::GetCurrentProjectionMatrix();
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
Material::Material(const std::string& name, const std::string& vertShaderSrc, const std::string& fragShaderSrc) : name(name) {
	//check if material is already loaded
	if (MaterialManager::HasMaterialByName(name)) {
		throw runtime_error("Material with name " + name + "is already allocated! Use GetMaterialByName to get it.");
	}
	
	//get all shader files for this programs
	string dir = "shaders/" + name + ".tar";
	
	if (!shaderfiles.exists(dir)){
		throw runtime_error("Material at path " + dir + " does not exist.");
	}
	
	//unpack the tar with the shaders
	auto file = shaderfiles.open(dir);
	stringstream input;
	for(const char c : file){
		input << c;
	}
	TarReader reader(input);
	
	//must have a vertex and a fragment shader
	bgfx::ShaderHandle vsh = loadShader(reader.GetFile("vertex.bin"));
	bgfx::ShaderHandle fsh = loadShader(reader.GetFile("fragment.bin"));
	program = bgfx::createProgram(vsh, fsh, true);
	if (!bgfx::isValid(program)){
		throw runtime_error("Material is invalid.");
	}

	//register material
	MaterialManager::RegisterMaterial(this);
}

Material::Material() : Material("cubes","","") {}

Material::~Material() {
}

/**
@returns if a material with the given name has been loaded.
@param the name of the material to find

*/
bool RavEngine::MaterialManager::HasMaterialByName(const std::string& name)
{
	mtx.lock();
	bool has = materials.find(name) != materials.end();
	mtx.unlock();
	return has;
}

/**
Mark a material for deletion by name. The material will remain allocated until its last reference is released.
@param name the name of the material to mark for deletion
*/
void RavEngine::MaterialManager::UnregisterMaterialByName(const std::string& name)
{
	mtx.lock();
	materials.erase(name);
	mtx.unlock();
}

void RavEngine::MaterialManager::RegisterMaterial(Ref<Material> mat)
{
	mtx.lock();
	materials.insert(std::make_pair(mat->GetName(), mat));
	mtx.unlock();
}
