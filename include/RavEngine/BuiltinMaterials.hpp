#pragma once
#include "Material.hpp"
#include "Uniform.hpp"
#include "Texture.hpp"

namespace RavEngine {
	class DefaultMaterial : public Material {
	public:
		DefaultMaterial() : Material("default") {}
	};

	class DefaultMaterialInstance : public MaterialInstance<DefaultMaterial> {
	public:
		DefaultMaterialInstance(Ref<DefaultMaterial> m) : MaterialInstance(m) { };

		void SetAlbedoTexture(Ref<Texture> texture) {
			albedo = texture;
		}

		void DrawHook() override {
			if (!albedo.isNull()) {
				albedo->Bind(0, albedoTxUniform);
			}
		}
	protected:
		SamplerUniform albedoTxUniform = SamplerUniform("s_albedoTex");
		Ref<Texture> albedo;
	};

	class DebugMaterial : public Material{
	public:
		DebugMaterial() : Material("debug"){};
	};

	class DebugMaterialInstance : public MaterialInstance<DebugMaterial>{
	public:
		DebugMaterialInstance(Ref<DebugMaterial> m ) : MaterialInstance(m){};		
	};
}
