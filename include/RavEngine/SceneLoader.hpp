#pragma once
#include <functional>
#include "Ref.hpp"
#include "mathtypes.hpp"

struct aiScene;

namespace RavEngine {
	class Entity;
	class MeshAsset;

	struct PreloadedAsset {
		std::string_view name;
	};

	struct PreloadedLight : public PreloadedAsset {
		// light details here...
	};
 
	struct Locator : public PreloadedAsset {
		vector3 translate, scale;
		quaternion rotation;
	};

	struct SceneLoader{

		/**
		* Construct a SceneLoader. This will load the scene file into an intermediate representation.
		*/
		SceneLoader(const std::string& sceneFile);

		/**
		* Unloads internal representation
		*/
		~SceneLoader();

		/**
		* Load the meshes for this scene
		* @param filterFunc the function to invoke to filter items. Return true if the mesh should be loaded, false to skip
		* @param constructionFunc the funciton to invoke with the created MeshAssets.
		*/
		void LoadMeshes(const std::function<bool(const PreloadedAsset&)>& filterFunc, const std::function<void(Ref<MeshAsset>, const PreloadedAsset&)>& constructionFunc);

		/**
		* Load the scene nodes for this scene
		* @param func the function to invoke with each node
		*/
		void LoadLocators(const std::function<void(const Locator&)>& func);

	private:
		const aiScene* scene;
	};


}