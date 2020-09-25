#include "MeshAsset.hpp"
#include "Common3D.hpp"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/mesh.h>
#include <vector>
#include "mathtypes.hpp"

using namespace RavEngine;

// Vertex data structure
using namespace std;

static const Vertex vertices[] =
{
	{-1.0f,  1.0f,  1.0f, 0xff000000 },
	{ 1.0f,  1.0f,  1.0f, 0xff0000ff },
	{-1.0f, -1.0f,  1.0f, 0xff00ff00 },
	{ 1.0f, -1.0f,  1.0f, 0xff00ffff },
	{-1.0f,  1.0f, -1.0f, 0xffff0000 },
	{ 1.0f,  1.0f, -1.0f, 0xffff00ff },
	{-1.0f, -1.0f, -1.0f, 0xffffff00 },
	{ 1.0f, -1.0f, -1.0f, 0xffffffff },
};

static const uint16_t indices[] = {
	2, 1, 0, // 0
	2, 3, 1,
	5, 6, 4, // 2
	7, 6, 5,
	4, 2, 0, // 4
	6, 2, 4,
	3, 5, 1, // 6
	3, 7, 5,
	1, 4, 0, // 8
	1, 5, 4,
	6, 3, 2, // 10
	7, 3, 6,
};



MeshAsset::MeshAsset(){
	//uses a meta-flag to auto-triangulate the input file
	const aiScene* scene = aiImportFile("orb.obj", aiProcessPreset_TargetRealtime_MaxQuality);
	
	if (!scene){
		throw runtime_error(string("cannot load: ") + aiGetErrorString());
	}
	
	//generate the vertex and index lists
	
	struct MeshPart{
		vector<uint16_t> indices;
		vector<Vertex> vertices;
	};
	
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
			mp.vertices.push_back({vert.x,vert.y,vert.z,0xff00ff00});
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
