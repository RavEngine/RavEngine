#include "MeshAsset.hpp"
#include "Common3D.hpp"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/mesh.h>
#include "App.hpp"
#include <filesystem>
#include "Debug.hpp"
#include "VirtualFileSystem.hpp"
#include "RenderEngine.hpp"
#include <RGL/Buffer.hpp>
#include <RGL/Device.hpp>

using namespace RavEngine;

// Vertex data structure
using namespace std;

static constexpr auto assimp_flags =  aiProcess_CalcTangentSpace |
aiProcess_GenSmoothNormals              |
aiProcess_FlipUVs |
aiProcess_JoinIdenticalVertices         |
aiProcess_ImproveCacheLocality          |
aiProcess_LimitBoneWeights              |
aiProcess_RemoveRedundantMaterials      |
aiProcess_SplitLargeMeshes              |
aiProcess_Triangulate                   |
aiProcess_GenUVCoords                   |
aiProcess_SortByPType                   |
//aiProcess_FindDegenerates               |
aiProcess_FindInstances                  |
aiProcess_ValidateDataStructure          |
aiProcess_OptimizeMeshes				|
aiProcess_FindInvalidData     ;

static const aiScene* LoadScene(const std::string& name){
	string dir = StrFormat("objects/{}", name);
	
	if (!GetApp()->GetResources().Exists(dir.c_str())) {
		Debug::Fatal("Cannot open resource: {}", dir);
	}
	
	auto str = GetApp()->GetResources().FileContentsAt(dir.c_str());
	
	auto file_ext = Filesystem::Path(dir).extension();
	//uses a meta-flag to auto-triangulate the input file
	const aiScene* scene = aiImportFileFromMemory(reinterpret_cast<char*>(str.data()), Debug::AssertSize<unsigned int>(str.size()),
												  assimp_flags,
												  file_ext.string().c_str());
	
	
	if (!scene){
		Debug::Fatal("Cannot load: {}", aiGetErrorString());
	}
	return scene;
}

static const aiScene* LoadSceneFilesystem(const Filesystem::Path& path){
	const aiScene* scene = aiImportFile(path.string().c_str(), assimp_flags);
	
	if (!scene){
		Debug::Fatal("Cannot load from filesystem: {}", aiGetErrorString());
	}
	return scene;
}

#if 0
static MeshAsset::BitWidth GetRequiredBitwidth(const aiScene* scene, const aiNode* rootNode = nullptr) {
	uint32_t totalVertCount = 0;
	if (rootNode == nullptr) {
		rootNode = scene->mRootNode;
	}
	for (int i = 0; i < rootNode->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[rootNode->mMeshes[i]];
		totalVertCount += mesh->mNumVertices;
	}
	// can it fit in a 16 bit index buffer?
	if (totalVertCount >= std::numeric_limits<uint16_t>::max()) {
		return MeshAsset::BitWidth::uint32;
	}
	else {
		return MeshAsset::BitWidth::uint16;
	}
}
#endif

void MeshAsset::InitAll(const aiScene* scene, const MeshAssetOptions& options){
	matrix4 scalemat = glm::scale(matrix4(1), vector3(options.scale,options.scale,options.scale));
	//indexBufferWidth = GetRequiredBitwidth(scene);
	//generate the vertex and index lists
	RavEngine::Vector<MeshPart> meshes;
	meshes.reserve(scene->mNumMeshes);
	for(int i = 0; i < scene->mNumMeshes; i++){
		aiMesh* mesh = scene->mMeshes[i];
		auto mp = AIMesh2MeshPart(mesh, scalemat);
		meshes.push_back(std::move(mp));
	}
	
	//free afterward
	aiReleaseImport(scene);
	
	InitializeFromMeshPartFragments(meshes, options);
}

void MeshAsset::InitPart(const aiScene* scene, const std::string& meshName, const std::string& fileName, const MeshAssetOptions& options){
	matrix4 scalemat = glm::scale(matrix4(1), vector3(options.scale,options.scale,options.scale));
	auto node = scene->mRootNode->FindNode(meshName.c_str());
	if (node == nullptr){
		Debug::Fatal("No mesh with name {} in scene {}",meshName, fileName);
	}
	else{
		//indexBufferWidth = GetRequiredBitwidth(scene,node);
		RavEngine::Vector<MeshPart> meshes;
		meshes.reserve(node->mNumMeshes);
		for(int i = 0; i < node->mNumMeshes; i++){
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			auto mp = AIMesh2MeshPart(mesh, scalemat);
			meshes.push_back(std::move(mp));
		}
		//free afterward
		aiReleaseImport(scene);
		InitializeFromMeshPartFragments(meshes, options);
	}
}

MeshAsset::MeshAsset(const string& name, const MeshAssetOptions& options){
	auto scene = LoadScene(name);

	InitAll(scene, options);
#if 0
	bgfx::setName(vertexBuffer, fmt::format("MeshAsset {} VB", name).c_str());
	bgfx::setName(indexBuffer, fmt::format("MeshAsset {} IB", name).c_str());
#endif
}

MeshAsset::MeshAsset(const Filesystem::Path& path, const MeshAssetOptions& opt){
	auto scene = LoadSceneFilesystem(path);
	
	InitAll(scene,opt);
}

MeshAsset::MeshAsset(const Filesystem::Path& path, const std::string& name, const MeshAssetOptions& opt){
	auto scene = LoadSceneFilesystem(path);
	
	InitPart(scene, name, path.string(), opt);
#if 0
	bgfx::setName(vertexBuffer, fmt::format("MeshAsset-FS {} VB", name).c_str());
	bgfx::setName(indexBuffer, fmt::format("MeshAsset-FS {} IB", name).c_str());
#endif
}

RavEngine::MeshAsset::~MeshAsset()
{
	if (auto app = GetApp()) {
		auto& gcBuffers = app->GetRenderEngine().gcBuffers;
		gcBuffers.enqueue(vertexBuffer);
		gcBuffers.enqueue(indexBuffer);
		app->GetRenderEngine().DeallocateMesh(meshAllocation);
	}
	
}

MeshAsset::MeshAsset(const string& name, const string& meshName, const MeshAssetOptions& options){
	auto scene = LoadScene(name);
	
	InitPart(scene, meshName, name, options);
#if 0
	bgfx::setName(vertexBuffer, fmt::format("MeshAsset-FS {} ({}) VB", meshName, name).c_str());
	bgfx::setName(indexBuffer, fmt::format("MeshAsset-FS {} ({}) IB", meshName, name).c_str());
#endif
}


MeshAsset::MeshPart RavEngine::MeshAsset::AIMesh2MeshPart(const aiMesh* mesh, const matrix4& scalemat)
{
	MeshPart mp;
	//mp.indices.mode = indexBufferWidth;

	mp.indices.reserve(mesh->mNumFaces * 3);
	mp.vertices.reserve(mesh->mNumVertices);
	for (int vi = 0; vi < mesh->mNumVertices; vi++) {
		auto vert = mesh->mVertices[vi];
		vector4 scaled(vert.x, vert.y, vert.z, 1);

		scaled = scalemat * scaled;

		auto normal = mesh->mNormals[vi];

		//does mesh have uvs?
		float uvs[2] = { 0 };
		if (mesh->mTextureCoords[0]) {
			uvs[0] = mesh->mTextureCoords[0][vi].x;
			uvs[1] = mesh->mTextureCoords[0][vi].y;
		}

		mp.vertices.push_back({
			static_cast<float>(scaled.x),static_cast<float>(scaled.y),static_cast<float>(scaled.z),	//coordinates
			normal.x,normal.y,normal.z,																//normals
			uvs[0],uvs[1]																			//UVs
			});
	}

	for (int ii = 0; ii < mesh->mNumFaces; ii++) {
		//alert if encounters a degenerate triangle
		if (mesh->mFaces[ii].mNumIndices != 3) {
			throw runtime_error("Cannot load model: Degenerate triangle (Num indices = " + to_string(mesh->mFaces[ii].mNumIndices) + ")");
		}

		mp.indices.push_back(mesh->mFaces[ii].mIndices[0]);
		mp.indices.push_back(mesh->mFaces[ii].mIndices[1]);
		mp.indices.push_back(mesh->mFaces[ii].mIndices[2]);

	}
	return mp;
}

void MeshAsset::InitializeFromMeshPartFragments(const RavEngine::Vector<MeshPart>& meshes, const MeshAssetOptions& options){
	//combine all meshes
	decltype(totalVerts) tv = 0;
	decltype(totalIndices) ti = 0;
	for(int i = 0; i < meshes.size(); i++){
		tv += meshes[i].vertices.size();
		ti += meshes[i].indices.size();
	}
	
	MeshPart allMeshes;
	allMeshes.vertices.reserve(tv);
	//allMeshes.indices.mode = indexBufferWidth;
	allMeshes.indices.reserve(ti);
	
	uint32_t baseline_index = 0;
	for(const auto& mesh : meshes){
		for(const auto& vert : mesh.vertices){
			allMeshes.vertices.push_back(vert);
		}
		for (int i = 0; i < mesh.indices.size(); i++) {
			allMeshes.indices.push_back(mesh.indices[i] + baseline_index);	//must recompute index here
		}
		baseline_index += mesh.vertices.size();
	}
	InitializeFromRawMesh(allMeshes, options);
}

void MeshAsset::InitializeFromRawMesh(const MeshPart& allMeshes, const MeshAssetOptions& options){
    if (options.keepInSystemRAM){
        systemRAMcopy = allMeshes;
    }
    InitializeFromRawMeshView(allMeshes,options);
}

void MeshAsset::InitializeFromRawMeshView(const MeshPartView& allMeshes, const MeshAssetOptions& options){
    
    // calculate bounding box
    for(const auto& vert : allMeshes.vertices){
        bounds.max[0] = std::max<decimalType>(bounds.max[0],vert.position[0]);
        bounds.max[1] = std::max<decimalType>(bounds.max[1],vert.position[1]);
        bounds.max[2] = std::max<decimalType>(bounds.max[2],vert.position[2]);
        
        bounds.min[0] = std::min<decimalType>(bounds.min[0],vert.position[0]);
        bounds.min[1] = std::min<decimalType>(bounds.min[1],vert.position[1]);
        bounds.min[2] = std::min<decimalType>(bounds.min[2],vert.position[2]);
    }
    
    if (options.uploadToGPU){
	
        //copy out of intermediate
        auto& v = allMeshes.vertices;
        auto& i = allMeshes.indices;
        totalVerts = v.size();
        totalIndices = i.size();

		auto device = GetApp()->GetDevice();

		vertexBuffer = device->CreateBuffer({
			uint32_t(totalVerts),
			{.VertexBuffer = true},
			sizeof(decltype(allMeshes.vertices)::value_type),
			RGL::BufferAccess::Private,
			{.Writable = false},
		});

		uint32_t index_stride = sizeof(uint32_t);

		indexBuffer = device->CreateBuffer({
			uint32_t(totalIndices),
			{.IndexBuffer = true},
			index_stride,
			RGL::BufferAccess::Private,
			{.Writable = false},
		});

		vertexBuffer->SetBufferData({ v.data(),v.size_bytes() });
		indexBuffer->SetBufferData({ i.data(), i.size_bytes() });

		meshAllocation = GetApp()->GetRenderEngine().AllocateMesh(v, i);
    }
}
