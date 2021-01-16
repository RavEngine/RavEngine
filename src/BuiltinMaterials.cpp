#include "BuiltinMaterials.hpp"
#include <bgfx/bgfx.h>
#include <glm/gtc/type_ptr.hpp>

using namespace RavEngine;

void PBRMaterialInstance::DrawHook(){
	if (!albedo) {
		albedo = TextureManager::defaultTexture;
	}
	albedo->Bind(0, mat->albedoTxUniform);
		
	mat->albedoColorUniform.SetValues(&color, 1);
}

void GUIMaterialInstance::DrawHook(){
	bgfx::setTexture(0, mat->sampler, mat->texture);
}
