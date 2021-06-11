#pragma once
#include "Material.hpp"
#include "Uniform.hpp"
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
		PBRMaterial(const std::string& name) : Material(name){}
        SamplerUniform albedoTxUniform = SamplerUniform("s_albedoTex");
        Vector4Uniform albedoColorUniform = Vector4Uniform("albedoColor");
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
       inline void SetAlbedoColor(const ColorRGBA& c){
            color = c;
        }

        virtual void DrawHook() override;
	protected:
		Ref<Texture> albedo = TextureManager::defaultTexture;
		ColorRGBA color{1,1,1,1};
	};

    /**
     Used internally for debug primitives
     */
	class DebugMaterial : public Material{
	public:
		DebugMaterial() : Material("debug"){};
	};
    /**
     Used internally for debug primitives
     */
	class DebugMaterialInstance : public MaterialInstance<DebugMaterial>{
	public:
		DebugMaterialInstance(Ref<DebugMaterial> m ) : MaterialInstance(m){};		
	};

    class DeferredBlitShader : public Material{
    public:
        DeferredBlitShader() : Material("deferred_blit"){}
    };

	/**
	 Used internally for rendering GUI
	 */
	class GUIMaterial : public Material{
	public:
		GUIMaterial() : Material("guishader"){}
	protected:
		SamplerUniform sampler = SamplerUniform("s_uitex");
		bgfx::TextureHandle texture;
		friend class GUIMaterialInstance;
	};

	class GUIMaterialInstance : public MaterialInstance<GUIMaterial>{
	public:
		GUIMaterialInstance(Ref<GUIMaterial> m) : MaterialInstance(m){}
		void SetTexture(bgfx::TextureHandle texture){
			mat->texture = texture;
		}
		
		void DrawHook() override;
	};
}
