#pragma once
#include "Material.hpp"
#include "Common3D.hpp"

namespace RavEngine {
    /**
     PBR material surface shader.
     Subclass this material to make custom surface shaders
     */
	class PBRMaterial : public Material {
	public:
		PBRMaterial(const std::string& name);
		PBRMaterial() : PBRMaterial("pbr") {}
	};

    /**
     Allows attaching a PBR material to an object.
     Subclass to expose additional fields in a custom shader
     */
	class PBRMaterialInstance : public MaterialInstance<PBRMaterial> {
	public:
		struct PushConstantData {
			ColorRGBA color{ 1,1,1,1 };
		};
		PBRMaterialInstance(Ref<PBRMaterial> m);;

		inline void SetAlbedoTexture(Ref<Texture> texture) {
			textureBindings[0] = texture;
		}
        constexpr inline void SetAlbedoColor(const ColorRGBA& c){
            pcd.color = c;
        }

		virtual const RGL::untyped_span GetPushConstantData() const override {
			return pcd;
		}

	protected:
		PushConstantData pcd;
	};
}
