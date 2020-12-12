#include "MeshAsset.hpp"
#include "Common3D.hpp"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/mesh.h>
#include <vector>
#include <random>
#include "App.hpp"
#include <filesystem>

using namespace RavEngine;

// Vertex data structure
using namespace std;

locked_hashmap<std::string, Ref<MeshAsset>> MeshAsset::Manager::meshes;

typedef VertexNormalUV vertex_t;

MeshAsset::MeshAsset(const string& name, const decimalType scale){
	string dir = "/meshes/" + name;
	
	if (!App::Resources->Exists(dir.c_str())) {
		throw runtime_error("Cannot open resource: " + dir);
	}

	auto str = App::Resources->FileContentsAt(dir.c_str());
	
	//pull from cmrc
	auto file_ext = filesystem::path(dir).extension();
	//uses a meta-flag to auto-triangulate the input file
	const aiScene* scene = aiImportFileFromMemory(str.data(), str.size(), aiProcessPreset_TargetRealtime_MaxQuality, file_ext.string().c_str());
	
	
	if (!scene){
		throw runtime_error(string("cannot load: ") + aiGetErrorString());
	}
	
	//generate the vertex and index lists
	
	struct MeshPart{
		vector<uint16_t> indices;
		vector<vertex_t> vertices;
	};
    
	matrix4 scalemat = glm::scale(matrix4(1), vector3(scale,scale,scale));
	
	vector<MeshPart> meshes;
	meshes.reserve(scene->mNumMeshes);
	for(int i = 0; i < scene->mNumMeshes; i++){
		aiMesh* mesh = scene->mMeshes[i];
		MeshPart mp;
		mp.indices.reserve(mesh->mNumFaces * 3);
		mp.vertices.reserve(mesh->mNumVertices);
		for(int vi = 0; vi < mesh->mNumVertices; vi++){
			auto vert = mesh->mVertices[vi];
			vector4 scaled(vert.x,vert.y,vert.z,1);
			
			scaled = scalemat * scaled;
			
			auto normal = mesh->mNormals[vi];
			
			//does mesh have uvs?
			float uvs[2] = {0};
			if(mesh->mTextureCoords[0]){
				uvs[0] = mesh->mTextureCoords[0][vi].x;
				uvs[1] = mesh->mTextureCoords[0][vi].y;
			}
			
			mp.vertices.push_back({
				static_cast<float>(scaled.x),static_cast<float>(scaled.y),static_cast<float>(scaled.z),	//coordinates
				normal.x,normal.y,normal.z,																//normals
				uvs[0],uvs[1]																			//UVs
			});
		}
		
		for(int ii = 0; ii < mesh->mNumFaces; ii++){
			//alert if encounters a degenerate triangle
			assert(mesh->mFaces[ii].mNumIndices == 3);
			
			mp.indices.push_back(mesh->mFaces[ii].mIndices[0]);
			mp.indices.push_back(mesh->mFaces[ii].mIndices[1]);
			mp.indices.push_back(mesh->mFaces[ii].mIndices[2]);

		}
		
		meshes.push_back(mp);
	}
	
	//free afterward
	aiReleaseImport(scene);
	
	
	//copy out of intermediate
	auto v = meshes[0].vertices;
	auto i = meshes[0].indices;
	
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
	
	auto ibm = bgfx::copy(&i[0], i.size() * sizeof(uint16_t));
	indexBuffer = bgfx::createIndexBuffer(ibm);

	if(! bgfx::isValid(vertexBuffer) || ! bgfx::isValid(indexBuffer)){
		throw runtime_error("Buffers could not be created.");
	}
	
	Manager::RegisterMeshAsset(name, this);
}

bool MeshAsset::Manager::IsMeshAssetLoaded(const std::string& path){
	bool result;
	result = meshes.find(path) != meshes.end();
	return result;
}

Ref<MeshAsset> MeshAsset::Manager::GetLoadedMeshAsset(const std::string& path){
	Ref<MeshAsset> result;
	result.setNull();
	try{
		result = meshes.at(path);
	}
	catch(exception& e){}
	return result;
}

void MeshAsset::Manager::RemoveMeshAsset(const std::string& path){
	meshes.erase(path);
}

void MeshAsset::Manager::RegisterMeshAsset(const std::string& path, Ref<MeshAsset> m){
	meshes[path] = m;
}
