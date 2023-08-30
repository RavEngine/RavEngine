#pragma once
#include "Function.hpp"
#include "Ref.hpp"
#include "mathtypes.hpp"
#include "Filesystem.hpp"

struct aiScene;

namespace RavEngine {
	class Entity;
	class MeshAsset;
	class PBRMaterialInstance;

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
        SceneLoader(const char* sceneFile);
		/**
		* Construct a SceneLoader. This will load the scene file into an intermediate representation.
		*/
		SceneLoader(const std::string& sceneFile);
		
		/**
		 Construct a SceneLoader from the user's filesystem. This will load the scene file into an intermediate representation
		 */
        SceneLoader(const Filesystem::Path& pathOnDisk);

		/**
		* Unloads internal representation
		*/
		~SceneLoader();
#if !RVE_SERVER
		/**
		* Load the meshes for this scene
		* @param filterFunc the function to invoke to filter items. Return true if the mesh should be loaded, false to skip
		* @param constructionFunc the funciton to invoke with the created MeshAssets.
		*/
		void LoadMeshes(const Function<bool(const PreloadedAsset&)>& filterFunc, const Function<void(Ref<MeshAsset>, Ref<PBRMaterialInstance>, const PreloadedAsset&)>& constructionFunc);
#endif
		/**
		* Load the scene nodes for this scene
		* @param func the function to invoke with each node
		*/
		void LoadLocators(const Function<void(const Locator&)>& func);

	private:
		const aiScene* scene;
		const std::string scene_path;
	};


}
