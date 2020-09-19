
#include "Material.hpp"
#include <sstream>
#include <fstream>
#include <RenderEngine.hpp>
#include "mathtypes.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <filesystem>
#include "Common3D.hpp"
#include <bgfx/bgfx.h>

using namespace std;
using namespace RavEngine;
using namespace std::filesystem;

MaterialManager::MaterialStore MaterialManager::materials;
matrix4 MaterialManager::projectionMatrix;
matrix4 MaterialManager::viewMatrix;
mutex MaterialManager::mtx;

// mapping names to types
const unordered_map<string, ShaderStage> stagemap{
	{"VS",ShaderStage::Vertex},
	{"FS",ShaderStage::Fragment},
	{"G",ShaderStage::Geometry},
	{"TE",ShaderStage::TessEval},
	{"TC",ShaderStage::TessControl},
	{"C",ShaderStage::Compute},
	
};

void RavEngine::Material::SetTransformMatrix(const matrix4& mat)
{
    transformMatrix = mat;
}

bgfx::ShaderHandle loadShader(const std::string filename){
	ifstream fin(filename, ios::binary);
	ostringstream data;
	data << fin.rdbuf();
	auto str = data.str();
	
	const bgfx::Memory* mem = bgfx::copy(str.c_str(), str.size());
	
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
		throw new runtime_error("Material with name " + name + "is already allocated! Use GetMaterialByName to get it.");
	}
	
	//load from folder -> file
	bgfx::ShaderHandle vsh = loadShader("vs_cubes.bin");
	bgfx::ShaderHandle fsh = loadShader("fs_cubes.bin");
	program = bgfx::createProgram(vsh, fsh, true);

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
