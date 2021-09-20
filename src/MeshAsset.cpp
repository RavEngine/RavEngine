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
	
	if (!App::Resources->Exists(dir.c_str())) {
		Debug::Fatal("Cannot open resource: {}", dir);
	}
	
	auto str = App::Resources->FileContentsAt(dir.c_str());
	
	auto file_ext = filesystem::path(dir).extension();
	//uses a meta-flag to auto-triangulate the input file
	const aiScene* scene = aiImportFileFromMemory(str.data(), str.size(),
												  assimp_flags,
												  file_ext.string().c_str());
	
	
	if (!scene){
		Debug::Fatal("Cannot load: {}", aiGetErrorString());
	}
	return scene;
}

MeshAsset::MeshAsset(const string& name, const MeshAssetOptions& options){
	auto scene = LoadScene(name);

	matrix4 scalemat = glm::scale(matrix4(1), vector3(options.scale,options.scale,options.scale));
	
	//generate the vertex and index lists
	vector<MeshPart> meshes;
	meshes.reserve(scene->mNumMeshes);
	for(int i = 0; i < scene->mNumMeshes; i++){
		aiMesh* mesh = scene->mMeshes[i];
		auto mp = AIMesh2MeshPart(mesh, scalemat);
		meshes.push_back(mp);
	}
	
	//free afterward
	aiReleaseImport(scene);
	
	InitializeFromMeshPartFragments(meshes, options);
}

MeshAsset::MeshAsset(const string& name, const string& meshName, const MeshAssetOptions& options){
	auto scene = LoadScene(name);
	
	matrix4 scalemat = glm::scale(matrix4(1), vector3(options.scale,options.scale,options.scale));
	
	auto node = scene->mRootNode->FindNode(meshName.c_str());
	if (node == nullptr){
		Debug::Fatal("No mesh with name {} in scene {}",meshName, name);
	}
	else{
		vector<MeshPart> meshes;
		meshes.reserve(node->mNumMeshes);
		for(int i = 0; i < node->mNumMeshes; i++){
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			auto mp = AIMesh2MeshPart(mesh, scalemat);
			meshes.push_back(mp);
		}
		InitializeFromMeshPartFragments(meshes, options);
	}
}

MeshAsset::MeshPart RavEngine::MeshAsset::AIMesh2MeshPart(const aiMesh* mesh, const matrix4& scalemat)
{
	MeshPart mp;
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

void MeshAsset::InitializeFromMeshPartFragments(const std::vector<MeshPart>& meshes, const MeshAssetOptions& options){
	//combine all meshes
	decltype(totalVerts) tv = 0;
	decltype(totalIndices) ti = 0;
	for(int i = 0; i < meshes.size(); i++){
		tv += meshes[i].vertices.size();
		ti += meshes[i].indices.size();
	}
	
	MeshPart allMeshes;
	allMeshes.vertices.reserve(tv);
	allMeshes.indices.reserve(ti);
	
	size_t baseline_index = 0;
	for(const auto& mesh : meshes){
		for(const auto& vert : mesh.vertices){
			allMeshes.vertices.push_back(vert);
		}
		for(const auto& index : mesh.indices){
			allMeshes.indices.push_back(index + baseline_index);	//must recompute index here
		}
		baseline_index += mesh.vertices.size();
	}
	InitializeFromRawMesh(allMeshes, options);
}

void MeshAsset::InitializeFromRawMesh(const MeshPart& allMeshes, const MeshAssetOptions& options){
	if (options.keepInSystemRAM){
		systemRAMcopy = allMeshes;
	}
    
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
        
        bgfx::VertexLayout pcvDecl;
        
        //vertex format
        pcvDecl.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float,true,true)
        .end();
        
        //create buffers
        auto vbm = bgfx::copy(&v[0], v.size() * sizeof(vertex_t));
        vertexBuffer = bgfx::createVertexBuffer(vbm, pcvDecl);
        
        auto ibm = bgfx::copy(&i[0], i.size() * sizeof(decltype(allMeshes.indices)::value_type));
        indexBuffer = bgfx::createIndexBuffer(ibm,BGFX_BUFFER_INDEX32);
        
        if(! bgfx::isValid(vertexBuffer) || ! bgfx::isValid(indexBuffer)){
            Debug::Fatal("Buffers could not be created.");
        }
    }
}
