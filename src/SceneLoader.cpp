#include "SceneLoader.hpp"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Debug.hpp"
#include "App.hpp"
#include "Utilities.hpp"
#include <filesystem>
#include "MeshAsset.hpp"

using namespace RavEngine;
using namespace std;

RavEngine::SceneLoader::SceneLoader(const std::string& name)
{
	auto dir = StrFormat("objects/{}",name);

	if (!App::Resources->Exists(dir.c_str())) {
		Debug::Fatal("Cannot open resource: {}", dir);
	}

	auto str = App::Resources->FileContentsAt(dir.c_str());

	auto file_ext = filesystem::path(dir).extension();
	//uses a meta-flag to auto-triangulate the input file
	scene = aiImportFileFromMemory(str.data(), str.size(),
		aiProcess_CalcTangentSpace |
		aiProcess_GenSmoothNormals |
		aiProcess_JoinIdenticalVertices |
		aiProcess_ImproveCacheLocality |
		aiProcess_LimitBoneWeights |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_SplitLargeMeshes |
		aiProcess_Triangulate |
		aiProcess_GenUVCoords |
		aiProcess_SortByPType |
		//aiProcess_FindDegenerates               |
		aiProcess_FindInstances |
		aiProcess_ValidateDataStructure |
		aiProcess_OptimizeMeshes |
		aiProcess_FindInvalidData,
		file_ext.string().c_str());


	if (!scene) {
		Debug::Fatal("Cannot load: {}", aiGetErrorString());
	}
}

RavEngine::SceneLoader::~SceneLoader()
{
	aiReleaseImport(scene);
}

void RavEngine::SceneLoader::LoadMeshes(const std::function<bool(const PreloadedAsset&)>& filterFunc, const std::function<void(Ref<MeshAsset>, const PreloadedAsset&)>& constructionFunc)
{
	matrix4 identity(1);
	for (decltype(scene->mNumMeshes) i = 0; i < scene->mNumMeshes; i++) {
		const auto& name = scene->mMeshes[i]->mName;
		PreloadedAsset pa{ string_view(name.C_Str(), name.length) };

		// user chooses if we load this mesh
		if (filterFunc(pa)) {
			auto mp = MeshAsset::AIMesh2MeshPart(scene->mMeshes[i],identity);
			auto asset = make_shared<MeshAsset>(mp);
			constructionFunc(asset, pa);
		}
	}
}

void RavEngine::SceneLoader::LoadLocators(const std::function<void(const Locator&)>& func)
{

}
