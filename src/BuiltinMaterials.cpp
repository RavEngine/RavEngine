#include "BuiltinMaterials.hpp"
#include <bgfx/bgfx.h>
#include <glm/gtc/type_ptr.hpp>

using namespace RavEngine;

void DefaultMaterialInstance::DrawHook(){
	if (albedo.isNull()) {
		albedo = TextureManager::defaultTexture;
	}
	albedo->Bind(0, mat->albedoTxUniform);
		
	mat->albedoColorUniform.SetValues(&color, 1);
}
