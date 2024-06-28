#pragma once
#if !RVE_SERVER
#include "Material.hpp"
#include "Common3D.hpp"
#include <variant>

namespace RavEngine {

	struct PBRPushConstantData {
		ColorRGBA color{ 1,1,1,1 };
		float metallicTint = 0;
		float roughnessTint = 0.4;
		float specularTint = 0.5;
	};
	
	struct PBRMaterial : public LitMaterial {
		PBRMaterial(LitMaterialOptions options = {.pushConstantSize = sizeof(PBRPushConstantData)}) : LitMaterial("pbr", options) {}
	};

	// the default layout and blend information.
	// you probably want these when defining custom materials.
	extern const decltype(MaterialConfig::vertConfig) defaultVertexConfig;
	extern const decltype(MaterialConfig::colorBlendConfig) defaultColorBlendConfig;
    extern const decltype(MaterialConfig::colorBlendConfig) defaultUnlitColorBlendConfig;

    /**
     Allows attaching a PBR material to an object.
     Don't subclass this if you have a custom material. Instead,
	 subclass LitMaterialInstance
     */

	class PBRMaterialInstance : public MaterialInstance {
	public:
		
		PBRMaterialInstance(Ref<PBRMaterial> m);

		inline void SetAlbedoTexture(Ref<Texture> texture) {
			textureBindings[1] = texture;
		}
		inline void SetNormalTexture(Ref<Texture> texture) {
			textureBindings[2] = texture;
		}
		inline void SetSpecularTexture(Ref<Texture> texture) {
			textureBindings[3] = texture;
		}
		inline void SetMetallicTexture(Ref<Texture> texture) {
			textureBindings[4] = texture;
		}
		inline void SetRoughnessTexture(Ref<Texture> texture) {
			textureBindings[5] = texture;
		}
		inline void SetAOTexture(Ref<Texture> texture) {
			textureBindings[6] = texture;
		}
        constexpr inline void SetAlbedoColor(const ColorRGBA& c){
            pushConstantData.color = c;
        }
		constexpr inline void SetMetallicTint(float c) {
			pushConstantData.metallicTint = c;
		}
		constexpr inline void SetSpecularTint(float c) {
			pushConstantData.specularTint = c;
		}
		constexpr inline void SetRoughnessTint(float c) {
			pushConstantData.roughnessTint = c;
		}

		virtual const RGL::untyped_span GetPushConstantData() const override {
			return pushConstantData;
		}
	private:
		PBRPushConstantData pushConstantData;
	};

   
}

#endif
