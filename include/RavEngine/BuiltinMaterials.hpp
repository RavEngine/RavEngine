#pragma once
#include "Material.hpp"
#include "Texture.hpp"
#include "Common3D.hpp"

namespace RavEngine {
    /**
     PBR material surface shader.
     Subclass this material to make custom surface shaders
     */
	class PBRMaterial : public Material {
	public:
		PBRMaterial() : Material("pbrmaterial") {}
		PBRMaterial(const std::string& name) : Material(name) {}
	};

    /**
     Allows attaching a PBR material to an object.
     Subclass to expose additional fields in a custom shader
     */
	class PBRMaterialInstance : public MaterialInstance<PBRMaterial> {
	public:
		PBRMaterialInstance(Ref<PBRMaterial> m) : MaterialInstance(m) { };

		inline void SetAlbedoTexture(Ref<Texture> texture) {
			albedo = texture;
		}
        constexpr inline void SetAlbedoColor(const ColorRGBA& c){
            color = c;
        }

        virtual void DrawHook() override;
	protected:
		Ref<Texture> albedo = TextureManager::defaultTexture;
		ColorRGBA color{1,1,1,1};
	};
}
