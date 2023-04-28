#include "BuiltinMaterials.hpp"
#include <glm/gtc/type_ptr.hpp>

using namespace RavEngine;

void PBRMaterialInstance::DrawHook(){
	if (!albedo) {
		albedo = TextureManager::defaultTexture;
	}
}
