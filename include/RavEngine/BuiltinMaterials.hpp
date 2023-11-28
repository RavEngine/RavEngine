#pragma once
#if !RVE_SERVER
#include "Material.hpp"
#include "Common3D.hpp"
#include <variant>

namespace RavEngine {
    /**
     PBR material surface shader.
     Subclass this material to make custom surface shaders
     */

	struct PBRMaterialOptions {
		RGL::CullMode cullMode = RGL::CullMode::Back;
	};

	struct PBRPushConstantData {
		ColorRGBA color{ 1,1,1,1 };
		float metallicTint = 0;
		float roughnessTint = 0.4;
		float specularTint = 0.5;
	};
	struct PBRMaterial : public Material {
		PBRMaterial(const std::string_view vsh_name, const std::string_view fsh_name, PBRMaterialOptions options = {});
		PBRMaterial(const std::string_view name, PBRMaterialOptions options = {}) : PBRMaterial(name, name, options) {}
		PBRMaterial(PBRMaterialOptions options = {}) : PBRMaterial("pbr", options) {}
	};

    struct UnlitMaterialOptions {
        RGL::CullMode cullMode = RGL::CullMode::Back;
    };

    // a material that reads no data
    struct UnlitMaterial : public Material{
        UnlitMaterial(const std::string_view vsh_name, const std::string_view fsh_name, UnlitMaterialOptions options = {});
        UnlitMaterial(const std::string_view name, UnlitMaterialOptions options = {}) : UnlitMaterial(name, name, options) {}
    };

	// the default layout and blend information.
	// you probably want these when defining custom materials.
	extern const decltype(MaterialConfig::vertConfig) defaultVertexConfig;
	extern const decltype(MaterialConfig::colorBlendConfig) defaultColorBlendConfig;
    extern const decltype(MaterialConfig::colorBlendConfig) defaultUnlitColorBlendConfig;

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

    struct UnlitMaterialInstance : public MaterialInstance {
        UnlitMaterialInstance(Ref<UnlitMaterial> mat) : MaterialInstance(mat){}
    };

    struct LitMeshMaterialInstance{
        Ref<MaterialInstance> material;
        LitMeshMaterialInstance() {}
        LitMeshMaterialInstance(const decltype(material)& material) : material(material){}
        inline bool operator==(const LitMeshMaterialInstance& other) const{
            return material == other.material;
        }
    };
    struct UnlitMeshMaterialInstance{
        Ref<MaterialInstance> material;
        UnlitMeshMaterialInstance() {}
        UnlitMeshMaterialInstance(const decltype(material)& material) : material(material){}
        inline bool operator==(const UnlitMeshMaterialInstance& other) const{
            return material == other.material;
        }
    };
    using MeshMaterial = std::variant<LitMeshMaterialInstance, UnlitMeshMaterialInstance>;
}

namespace std {
    template<>
    struct hash<RavEngine::LitMeshMaterialInstance> {
        size_t operator()(const RavEngine::LitMeshMaterialInstance& other) const {
            return std::hash<Ref<RavEngine::MaterialInstance>>()(other.material);
        }
    };

    template<>
    struct hash<RavEngine::UnlitMeshMaterialInstance> {
        size_t operator()(const RavEngine::UnlitMeshMaterialInstance& other) const {
            return std::hash<Ref<RavEngine::MaterialInstance>>()(other.material);
        }
    };
}
#endif
