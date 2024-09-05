#include "MeshAsset.hpp"
#include "Common3D.hpp"
#include "App.hpp"
#include <filesystem>
#include "Debug.hpp"
#include "VirtualFileSystem.hpp"
#if !RVE_SERVER
    #include "RenderEngine.hpp"
    #include <RGL/Buffer.hpp>
    #include <RGL/Device.hpp>
#endif

using namespace RavEngine;

// Vertex data structure
using namespace std;

MeshPart RavEngine::MeshAsset::DeserializeMesh(const istream& stream)
{
	return {};
}

MeshPart RavEngine::MeshAsset::DeserializeMeshFromMemory(const std::span<uint8_t> mem)
{
	uint8_t* fp = mem.data();
	SerializedMeshDataHeader header = *reinterpret_cast<SerializedMeshDataHeader*>(fp);
	fp += sizeof(SerializedMeshDataHeader);

	// check header
	if (strncmp(header.header.data(), "rvem", sizeof("rvem") - 1) != 0) {
		Debug::Fatal("Header does not match, data is not a mesh!");
	}

	MeshPart mesh;
	mesh.indices.reserve(header.numIndicies);
	mesh.vertices.reserve(header.numVertices);

	// load vertices
	for (int i = 0; i < header.numVertices; i++) {
		VertexNormalUV vert = *reinterpret_cast<decltype(vert)*>(fp);
		mesh.vertices.push_back(vert);

		fp += sizeof(VertexNormalUV);
	}

	for (int i = 0; i < header.numIndicies; i++) {
		uint32_t ind = *reinterpret_cast<decltype(ind)*>(fp);
		mesh.indices.push_back(ind);

		fp += sizeof(uint32_t);
	}

	return mesh;
}

MeshAsset::MeshAsset(const string& name, const MeshAssetOptions& options){
	string dir = Format("meshes/{}.rvem", name);
	auto str = GetApp()->GetResources().FileContentsAt(dir.c_str());

	auto mesh = DeserializeMeshFromMemory(str);
	InitializeFromMeshPartFragments({ mesh }, options);
}

MeshAsset::MeshAsset(const Filesystem::Path& path, const MeshAssetOptions& opt){
	
	auto mesh = DeserializeMesh(ifstream(path, std::ios::binary));
	InitializeFromMeshPartFragments({ mesh }, opt);
}


RavEngine::MeshAsset::~MeshAsset()
{
#if !RVE_SERVER
	if (auto app = GetApp()) {
		auto& gcBuffers = app->GetRenderEngine().gcBuffers;
		gcBuffers.enqueue(vertexBuffer);
		gcBuffers.enqueue(indexBuffer);
		app->GetRenderEngine().DeallocateMesh(meshAllocation);
	}
#endif
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
		for(auto vert : mesh.vertices){
			vert.position *= options.scale;			// apply scale 
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
        
        radius = std::max(radius, glm::distance(glm::vec3(vert.position[0],vert.position[1],vert.position[2]), glm::vec3(0,0,0)));
    }
    
    if (options.uploadToGPU){
	
        //copy out of intermediate
        auto& v = allMeshes.vertices;
        auto& i = allMeshes.indices;
        totalVerts = v.size();
        totalIndices = i.size();
#if !RVE_SERVER
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
#endif
    }
}
