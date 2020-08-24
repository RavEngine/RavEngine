
#include "Material.hpp"
#include <sstream>
#include <fstream>
#include <RenderEngine.hpp>

using namespace std;
using namespace RavEngine;

MaterialManager::MaterialStore MaterialManager::materials;
mutex MaterialManager::mtx;

string defaultMatPath() {
	auto path = "../deps/filament/filament/generated/material/defaultMaterial.filamat";
#ifdef _WIN32
	path += 3;
#endif
	return path;
}
/**
Create a material given a shader. Also registers it in the material manager
@param shader the path to the shader
*/
Material::Material(const std::string& shaderPath, const std::string& name) : name(name) {
	//check if material is already loaded
	if (MaterialManager::HasMaterialByName(name)) {
		throw new runtime_error("Material with name " + name + "is already allocated! Use GetMaterialByName to get it.");
	}

	//load material
	ifstream fin(shaderPath, ios::binary);
	assert(fin.good());	//ensure file exists
	ostringstream buffer;
	buffer << fin.rdbuf();
	auto shaderData = buffer.str();

	//create shader program

	//register material
	MaterialManager::RegisterMaterial(this);
}

Material::Material() : Material(defaultMatPath(),"defaultMaterial") {}

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
