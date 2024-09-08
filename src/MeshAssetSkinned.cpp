#include "MeshAssetSkinned.hpp"
#include "App.hpp"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/mesh.h>
#include "Filesystem.hpp"
#include "VirtualFileSystem.hpp"
#if !RVE_SERVER
    #include "RenderEngine.hpp"
    #include <RGL/Device.hpp>
    #include <RGL/Buffer.hpp>
#endif
#include "SkeletonAsset.hpp"
#include "Skeleton.hpp"

using namespace RavEngine;
using namespace std;

RavEngine::MeshAssetSkinned::~MeshAssetSkinned()
{
#if !RVE_SERVER
	if (auto app = GetApp()) {
		app->GetRenderEngine().gcBuffers.enqueue(weightsBuffer);
	}
#endif
}

//TODO: avoid opening the file twice -- this is a double copy and repeats work, therefore slow
MeshAssetSkinned::MeshAssetSkinned(const std::string& path, Ref<SkeletonAsset> skeleton, float scale) : MeshAsset(path,MeshAssetOptions{false,true}){
	
	auto fullpath = Format("objects/{}",path);
	
	if (!GetApp()->GetResources().Exists(fullpath.c_str())){
		Debug::Fatal("No asset at {}",fullpath);
	}
	
	auto str = GetApp()->GetResources().FileContentsAt(fullpath.c_str());
	
	//pull from cmrc
	auto file_ext = Filesystem::Path(fullpath).extension();
	//uses a meta-flag to auto-triangulate the input file
	const aiScene* scene = aiImportFileFromMemory(reinterpret_cast<char*>(str.data()), Debug::AssertSize<unsigned int>(str.size()),
												  aiProcess_CalcTangentSpace |
												  aiProcess_GenSmoothNormals              |
												  aiProcess_JoinIdenticalVertices         |
												  aiProcess_ImproveCacheLocality          |
												  aiProcess_LimitBoneWeights              |
												  aiProcess_RemoveRedundantMaterials      |
												  aiProcess_SplitLargeMeshes              |
												  aiProcess_Triangulate                   |
												  aiProcess_GenUVCoords                   |
												  aiProcess_SortByPType                   |
												  aiProcess_FindInstances                 |
												  aiProcess_ValidateDataStructure         |
												  aiProcess_OptimizeMeshes				  |
												  aiProcess_FindInvalidData     ,
												  file_ext.string().c_str());
	
	
	if (!scene){
		Debug::Fatal("Cannot load: {}", aiGetErrorString());
	}
	
	
#if !RVE_SERVER
	//make gpu version
    std::vector<VertexWeights> weightsgpu; //TODO: read from file

	//map to GPU
	weightsBuffer = GetApp()->GetDevice()->CreateBuffer({
		uint32_t(weightsgpu.size()),
		{.StorageBuffer = true},
		sizeof(VertexWeights),
		RGL::BufferAccess::Private,
		{.Writable = false}
	});
	weightsBuffer->SetBufferData({ weightsgpu.data(),weightsgpu.size() * sizeof(decltype(weightsgpu)::value_type)});
#endif
}

