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
	scene = aiImportFileFromMemory(reinterpret_cast<char*>(str.data()), str.size(),
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
	auto getWorldMatrix = [](const aiNode * node) -> aiMatrix4x4 {
		// figure out size
		uint16_t depth = 0;
		for (aiNode* p = node->mParent; p != nullptr; p = p->mParent) {
			++depth;
		}
		stackarray(transforms, aiMatrix4x4, depth);

		uint16_t tmp = 0;
		for (aiNode* p = node->mParent; p != nullptr; p = p->mParent) {
			transforms[tmp] = p->mTransformation;
			++tmp;
		}

		aiMatrix4x4 mat;	//identity
		if (depth > 0) {
			for (int i = depth - 1; i >= 0; --i) {
				mat *= transforms[i];
			}
		}
		mat *= node->mTransformation;
		return mat;
	};

	auto recurse_node = [&](aiNode* node) -> void {
		auto recurse_node_impl = [&](aiNode* node, auto& recursive_func) -> void {
			// process the node
			aiVector3t<float> scale, position;
			aiQuaterniont<float> rotation;
			getWorldMatrix(node).Decompose(scale, rotation, position);
			Locator l;
			l.name = std::string_view(node->mName.C_Str(), node->mName.length);
			l.translate = vector3(position.x, position.y, position.z);
			l.scale = vector3(scale.x, scale.y, scale.z);
			l.rotation = quaternion(rotation.w, rotation.x, rotation.y, rotation.z);

			func(l);

			for ( decltype(node->mNumChildren) i = 0; i < node->mNumChildren; i++) {
				recursive_func(node->mChildren[i], recursive_func);
			}
		};
		recurse_node_impl(node, recurse_node_impl);
	};
	recurse_node(scene->mRootNode);
}
