
#include "Material.hpp"
#include <sstream>
#include <fstream>

using namespace std;
using namespace RavEngine;

MaterialManager::MaterialStore MaterialManager::materials;
mutex MaterialManager::mtx;

/**
Create a material given a shader. Also registers it in the material manager
@param shader the path to the Filament shader
*/
Material::Material(const std::string& shader, const std::string& name) : name(name) {
	//load material

	//register material
	MaterialManager::RegisterMaterial(this);
}

Material::Material() : Material("path/to/defaultmat","defaultMaterial") {}

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
