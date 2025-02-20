#pragma once
#if !RVE_SERVER
#include "Material.hpp"
#include "Common3D.hpp"
#include <variant>
#include <string_view>

namespace RavEngine {

	struct PBRPushConstantData {
		ColorRGBA color{ 1,1,1,1 };
		float metallicTint = 0;
		float roughnessTint = 0.4;
		float specularTint = 0.5;
	};
	
	struct PBRMaterial : public LitMaterial {
		PBRMaterial(MaterialRenderOptions options = {  });
	protected:
		PBRMaterial(MaterialRenderOptions options, const std::string_view vsh_name, const std::string_view fsh_name);
	};

	struct PBRMaterialBaked : public PBRMaterial {
		PBRMaterialBaked(MaterialRenderOptions options = {  });
	};

	// the default layout and blend information.
	// you probably want these when defining custom materials.
	extern const decltype(MaterialConfig::vertConfig) defaultVertexConfig;
	extern const decltype(MaterialConfig::colorBlendConfig) defaultColorBlendConfig, defaultTransparentColorBlendConfig;
    extern const decltype(MaterialConfig::colorBlendConfig) defaultUnlitColorBlendConfig, defaultTransparentUnlitColorBlendConfig;

    /**
     Allows attaching a PBR material to an object.
     Don't subclass this if you have a custom material. Instead,
	 subclass LitMaterialInstance
     */

	class PBRMaterialInstance : public MaterialInstance {
	public:
		
		PBRMaterialInstance(Ref<PBRMaterial> m, uint32_t priority = 0);

		void SetAlbedoTexture(Ref<Texture> texture) {
			textureBindings[1] = texture;
		}
		void SetNormalTexture(Ref<Texture> texture) {
			textureBindings[2] = texture;
		}
		void SetSpecularTexture(Ref<Texture> texture) {
			textureBindings[3] = texture;
		}
		void SetMetallicTexture(Ref<Texture> texture) {
			textureBindings[4] = texture;
		}
		void SetRoughnessTexture(Ref<Texture> texture) {
			textureBindings[5] = texture;
		}
		void SetAOTexture(Ref<Texture> texture) {
			textureBindings[6] = texture;
		}
		void SetEmissiveTexture(Ref<Texture> texture) {
			textureBindings[7] = texture;
		}
        void SetAlbedoColor(const ColorRGBA& c){
            pushConstantData.color = c;
        }
		void SetMetallicTint(float c) {
			pushConstantData.metallicTint = c;
		}
		void SetSpecularTint(float c) {
			pushConstantData.specularTint = c;
		}
		void SetRoughnessTint(float c) {
			pushConstantData.roughnessTint = c;
		}

		virtual const RGL::untyped_span GetPushConstantData() const override {
			return pushConstantData;
		}
	private:
		PBRPushConstantData pushConstantData;
	};

	class PBRMaterialBakedInstance : public PBRMaterialInstance {
	public:
		PBRMaterialBakedInstance(Ref<PBRMaterialBaked> m, uint32_t priority = 0) : PBRMaterialInstance(m, priority) {}

		void SetBakedDirectionTexture(Ref<Texture> texture) {
			textureBindings[8] = texture;
		}

		void SetBakedEmissivityTexture(Ref<Texture> texture) {
			textureBindings[9] = texture;
		}
	};
}

#endif
