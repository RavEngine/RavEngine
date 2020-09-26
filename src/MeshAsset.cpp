#include "MeshAsset.hpp"
#include "Common3D.hpp"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/mesh.h>
#include <vector>
#include "mathtypes.hpp"
#include <cmrc/cmrc.hpp>
#include <sstream>
#include <filesystem>
#include <random>

using namespace RavEngine;

// Vertex data structure
using namespace std;

CMRC_DECLARE(RavEngine_RSC_Meshes);

static const cmrc::embedded_filesystem meshFiles = cmrc::RavEngine_RSC_Meshes::get_filesystem();

MeshAsset::MeshAsset(const string& name){
	string dir = "meshes/" + name;
	
	if (!meshFiles.exists(dir)){
		throw runtime_error("Material at path " + dir + " does not exist.");
	}
	
	//TODO: optimize this to not copy
	auto file = meshFiles.open(dir);
	string str;
	{
		stringstream input;
		for(const char c : file){
			input << c;
		}
		str = input.str();
	}
	
	//pull from cmrc
	auto file_ext = filesystem::path(dir).extension();
	//uses a meta-flag to auto-triangulate the input file
	const aiScene* scene = aiImportFileFromMemory(str.c_str(), str.size(), aiProcessPreset_TargetRealtime_MaxQuality, file_ext.string().c_str());
	
	
	if (!scene){
		throw runtime_error(string("cannot load: ") + aiGetErrorString());
	}
	
	//generate the vertex and index lists
	
	struct MeshPart{
		vector<uint16_t> indices;
		vector<Vertex> vertices;
	};
    
    std::random_device rd; // obtain a random number from hardware
   std::mt19937 gen(rd()); // seed the generator
   std::uniform_int_distribution<> distr(0, 16777215); // define the range
	
	vector<MeshPart> meshes;
	meshes.reserve(scene->mNumMeshes);
	for(int i = 0; i < scene->mNumMeshes; i++){
		aiMesh* mesh = scene->mMeshes[i];
		MeshPart mp;
		mp.indices.reserve(mesh->mNumFaces * 3);
		mp.vertices.reserve(mesh->mNumVertices);
		for(int vi = 0; vi < mesh->mNumVertices; vi++){
			auto vert = mesh->mVertices[vi];
			Vertex v;
            mp.vertices.push_back({vert.x,vert.y,vert.z,static_cast<uint32_t>(distr(gen))});
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
	
	bgfx::VertexLayout pcvDecl;
	
	//vertex format
	pcvDecl.begin()
	.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
	.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
	.end();
	
	//copy out of intermediate
	auto v = meshes[0].vertices;
	auto i = meshes[0].indices;
	auto vbm = bgfx::copy(&v[0], v.size() * sizeof(Vertex));
	auto ibm = bgfx::copy(&i[0], i.size() * sizeof(uint16_t));

	//create buffers
	vertexBuffer = bgfx::createVertexBuffer(vbm, pcvDecl);
	indexBuffer = bgfx::createIndexBuffer(ibm);

	
	if(! bgfx::isValid(vertexBuffer) || !bgfx::isValid(indexBuffer)){
		throw runtime_error("Buffers could not be created.");
	}
}
