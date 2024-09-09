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
MeshAssetSkinned::MeshAssetSkinned(const std::string& path) // we intentionally DO NOT call the constructor
{
	
	auto fullpath = Format("meshes/{}.rvem",path);
	
	if (!GetApp()->GetResources().Exists(fullpath.c_str())){
		Debug::Fatal("No asset at {}",fullpath);
	}
	
	auto str = GetApp()->GetResources().FileContentsAt(fullpath.c_str());


	auto mesh = DeserializeMeshFromMemory(str);
	InitializeFromRawMesh(mesh.first, MeshAssetOptions{ false,true });	// this intializes the staticmesh part
	
	
#if !RVE_SERVER
	// mesh.second has the file offset

	// check if there is weight data
	if (str.size() - mesh.second < 4) {
		Debug::Fatal("Mesh is probably not a skinned mesh");
	}

	uint8* fp = str.data() + mesh.second;
	auto size = ((str.data() + str.size()) - fp) / sizeof(VertexWeights);
	Debug::Assert(size == GetNumVerts(),"Skin does not have vertex weights for every vertex, input file is corrupt");

	//make gpu version
    std::vector<VertexWeights> weightsgpu;
	weightsgpu.reserve(size);
	for (int i = 0; i < size; i++) {
		VertexWeights weights = *(reinterpret_cast<VertexWeights*>(fp)+i);
		weightsgpu.push_back(weights);
	}

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

