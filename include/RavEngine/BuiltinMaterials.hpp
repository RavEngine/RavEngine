#pragma once
#include "Material.hpp"
#include "Uniform.hpp"
#include "Texture.hpp"

namespace RavEngine {
	class DefaultMaterial : public Material {
	public:
		DefaultMaterial() : Material("default") {}
        friend class DefaultMaterialInstance;
    protected:
        SamplerUniform albedoTxUniform = SamplerUniform("s_albedoTex");
        Vector4Uniform albedoColorUniform = Vector4Uniform("albedoColor");
	};

	class DefaultMaterialInstance : public MaterialInstance<DefaultMaterial> {
	public:
		DefaultMaterialInstance(Ref<DefaultMaterial> m) : MaterialInstance(m) { };

		void SetAlbedoTexture(Ref<Texture> texture) {
			albedo = texture;
		}
        void SetAlbedoColor(const vector4& c){
            color = c;
        }

        void DrawHook() override;
	protected:
		Ref<Texture> albedo = TextureManager::defaultTexture;
        vector4 color = vector4(1,1,1,1);
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
