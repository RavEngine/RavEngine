#pragma once
#include "Material.hpp"
#include "MeshAsset.hpp"

namespace RavEngine {
	
	// subclass
	struct ISkyMaterial : public Material {
		ISkyMaterial(const std::string& shaderpath) : Material(shaderpath) {}
	};

	// subclass
	struct ISkyMaterialInstance : public MaterialInstance<ISkyMaterial> {
		inline void Draw(const bgfx::VertexBufferHandle& vertexBuffer, const bgfx::IndexBufferHandle& indexBuffer, const matrix4& worldmatrix, int view = 0) override{
			float transmat[16];
			copyMat4((const decimalType*)glm::value_ptr(worldmatrix), transmat);
			bgfx::setTransform(transmat);
			mat->Draw(vertexBuffer, indexBuffer, view);
		}
		ISkyMaterialInstance(const Ref<ISkyMaterial> sm) : MaterialInstance(sm) {}
	};

	// default implementation, sublcass or implement your own

	struct DefaultSkyMaterial : public ISkyMaterial {
		DefaultSkyMaterial() : ISkyMaterial("defaultsky") {}
	};

	struct DefaultSkyMaterialInstance : public  ISkyMaterialInstance {
		void DrawHook() override {
			// set uniforms here...
		}
		DefaultSkyMaterialInstance(const Ref<ISkyMaterial> smi) : ISkyMaterialInstance(smi) {}
	};

	struct Skybox {
		bool enabled = true;
		Ref<ISkyMaterialInstance> skyMat;
		Ref<MeshAsset> skyMesh;
        inline void Draw(const matrix4& worldmatrix, int view) const{
			skyMat->Draw(skyMesh->getVertexBuffer(), skyMesh->getIndexBuffer(), worldmatrix, view);
		}

		// default constructor, loads default sky implementation
		Skybox();

		// supply a custom mesh and material
		Skybox(const decltype(skyMat)& sm, const decltype(skyMesh)& sme) : skyMat(sm), skyMesh(sme) {}

		friend class App;
	private:
		static Ref<MeshAsset> defaultSkyMesh;
		static void Init();
		static void Teardown();
	};

	
}
