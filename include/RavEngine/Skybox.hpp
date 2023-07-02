#pragma once
#include "Material.hpp"

namespace RavEngine {
	struct MeshAsset;
	
	// subclass
	struct ISkyMaterial : public Material {
		ISkyMaterial(const std::string& shaderpath);
	};

	// subclass
	struct ISkyMaterialInstance : public MaterialInstance {
		ISkyMaterialInstance(const Ref<ISkyMaterial> sm) : MaterialInstance(sm) {}
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
		Ref<MeshAsset> skyMesh;	// optional. If unset, the default sky mesh is rendered instead. 

		// default constructor, loads default sky implementation
		Skybox();

		// supply a custom mesh and material
		Skybox(const decltype(skyMat)& sm, const decltype(skyMesh)& sme = nullptr) : skyMat(sm), skyMesh(sme) {}

		friend class App;
	};

	
}
