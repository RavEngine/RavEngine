#pragma once
#include "Material.hpp"
#include "Common3D.hpp"

namespace RavEngine {
    /**
     PBR material surface shader.
     Subclass this material to make custom surface shaders
     */
	struct PBRPushConstantData {
		ColorRGBA color{ 1,1,1,1 };
	};
	class PBRMaterial : public Material {
	public:
		PBRMaterial(const std::string_view vsh_name, const std::string_view fsh_name);
		PBRMaterial(const std::string_view name) : PBRMaterial(name, name) {}
		PBRMaterial() : PBRMaterial("pbr") {}
	};

	// the default layout and blend information.
	// you probably want these when defining custom materials.
	extern const decltype(MaterialConfig::vertConfig) defaultVertexConfig;
	extern const decltype(MaterialConfig::colorBlendConfig) defaultColorBlendConfig;

    /**
     Allows attaching a PBR material to an object.
     Don't subclass this if you have a custom material. Instead,
	 subclass MaterialInstance<YourMaterialType>
     */

	class PBRMaterialInstance : public MaterialInstance {
	public:
		
		PBRMaterialInstance(Ref<PBRMaterial> m);

		inline void SetAlbedoTexture(Ref<Texture> texture) {
			textureBindings[1] = texture;
		}
        constexpr inline void SetAlbedoColor(const ColorRGBA& c){
            pushConstantData.color = c;
        }

		virtual const RGL::untyped_span GetPushConstantData() const override {
			return pushConstantData;
		}
	private:
		PBRPushConstantData pushConstantData;
	};
}
