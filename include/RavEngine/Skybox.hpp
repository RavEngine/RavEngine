#if !RVE_SERVER
#pragma once
#include "Material.hpp"

namespace RavEngine {
	struct MeshAsset;
	
	// subclass
	struct ISkyMaterial : public Material {
		ISkyMaterial(const std::string& shaderpath);
	};

	// subclass
	struct ISkyMaterialInstance {
		ISkyMaterialInstance(const Ref<ISkyMaterial> sm) : sm(sm) {}

		Ref<ISkyMaterial> GetMat() const {
			return sm;
		}

	private:
		Ref<ISkyMaterial> sm;
	};

	// default implementation, sublcass or implement your own

	struct DefaultSkyMaterial : public ISkyMaterial {
		DefaultSkyMaterial() : ISkyMaterial("defaultsky") {}
	};

	struct DefaultSkyMaterialInstance : public  ISkyMaterialInstance {
		DefaultSkyMaterialInstance(const Ref<ISkyMaterial> smi) : ISkyMaterialInstance(smi) {}
	};

	struct Skybox {
		bool enabled = true;
		Ref<ISkyMaterialInstance> skyMat;

		// default constructor, loads default sky implementation
		Skybox();

		// supply a custom mesh and material
		Skybox(const decltype(skyMat)& sm ): skyMat(sm){}

		friend class App;
	};

	
}
#endif
