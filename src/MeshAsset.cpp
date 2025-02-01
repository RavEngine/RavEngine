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

std::pair<MeshPart, uint32_t> RavEngine::MeshAsset::DeserializeMeshFromMemory(const std::span<uint8_t> mem)
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
	mesh.ReserveVerts(header.numVertices);
	
	auto loadMeshProperty = [&fp, &header](auto&& destination) {
		for (int i = 0; i < header.numVertices; i++) {
			std::remove_reference_t<decltype(destination[0])> prop = *reinterpret_cast<decltype(prop)*>(fp);
			destination.push_back(prop);
			fp += sizeof(prop);
		}
	};

	// load vertices
	if (header.attributes & SerializedMeshDataHeader::hasPositionsBit) {
		mesh.attributes.position = true;
		loadMeshProperty(mesh.positions);
	}
	if (header.attributes & SerializedMeshDataHeader::hasNormalsBit) {
		mesh.attributes.normal = true;
		loadMeshProperty(mesh.normals);
	}
	if (header.attributes & SerializedMeshDataHeader::hasTangentsBit) {
		mesh.attributes.tangent = true;
		loadMeshProperty(mesh.tangents);
	}
	if (header.attributes & SerializedMeshDataHeader::hasBitangentsBit) {
		mesh.attributes.bitangent = true;
		loadMeshProperty(mesh.bitangents);
	}
	if (header.attributes & SerializedMeshDataHeader::hasUV0Bit) {
		mesh.attributes.uv0 = true;
		loadMeshProperty(mesh.uv0);
	}
	if (header.attributes & SerializedMeshDataHeader::hasLightmapUVBit) {
		mesh.attributes.lightmapUV = true;
		loadMeshProperty(mesh.lightmapUVs);
	}

	for (int i = 0; i < header.numIndicies; i++) {
		uint32_t ind = *reinterpret_cast<decltype(ind)*>(fp);
		mesh.indices.push_back(ind);

		fp += sizeof(ind);
	}

	return { mesh, fp - mem.data()};
}

MeshAsset::MeshAsset(const string& name, const MeshAssetOptions& options){
	string dir = Format("meshes/{}.rvem", name);
	auto str = GetApp()->GetResources().FileContentsAt(dir.c_str());

	auto mesh = DeserializeMeshFromMemory(str);
	InitializeFromRawMesh(mesh.first, options);
}

MeshAsset::MeshAsset(const Filesystem::Path& path, const MeshAssetOptions& opt){
	
	auto mesh = DeserializeMesh(ifstream(path, std::ios::binary));
	InitializeFromRawMesh(mesh, opt);
}


RavEngine::MeshAsset::~MeshAsset()
{
#if !RVE_SERVER
	if (auto app = GetApp()) {
		app->GetRenderEngine().DeallocateMesh(meshAllocation);
	}
#endif
}

void MeshAsset::InitializeFromMeshPartFragments(const RavEngine::Vector<MeshPart>& meshes, const MeshAssetOptions& options){
	//combine all meshes
	decltype(totalVerts) tv = 0;
	decltype(totalIndices) ti = 0;
	for(int i = 0; i < meshes.size(); i++){
		tv += meshes[i].NumVerts();
		ti += meshes[i].indices.size();
	}
	
	MeshPart allMeshes;
	allMeshes.ReserveVerts(tv);
	//allMeshes.indices.mode = indexBufferWidth;
	allMeshes.indices.reserve(ti);
	
	MeshAttributes attrCheck = meshes[0].attributes;
	
	uint32_t baseline_index = 0;
	for(const auto& mesh : meshes){
		Debug::Assert(attrCheck == mesh.attributes, "Meshes have incompatible attributes!");

		auto copyProp = [](auto& destination, const auto& source) {
			for (const auto& prop : source) {
				destination.push_back(prop);
			}
		};
		copyProp(allMeshes.positions, mesh.positions);
		copyProp(allMeshes.normals, mesh.normals);
		copyProp(allMeshes.tangents, mesh.tangents);
		copyProp(allMeshes.bitangents, mesh.bitangents);
		copyProp(allMeshes.uv0, mesh.uv0);
		copyProp(allMeshes.lightmapUVs, mesh.lightmapUVs);
		
		for (int i = 0; i < mesh.indices.size(); i++) {
			allMeshes.indices.push_back(mesh.indices[i] + baseline_index);	//must recompute index here
		}
		baseline_index += mesh.NumVerts();
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
	attributes = allMeshes.attributes;

    // calculate bounding box
    for(const auto& pos : allMeshes.positions){
        bounds.max[0] = std::max<decimalType>(bounds.max[0],pos[0]);
        bounds.max[1] = std::max<decimalType>(bounds.max[1],pos[1]);
        bounds.max[2] = std::max<decimalType>(bounds.max[2],pos[2]);
        														  
        bounds.min[0] = std::min<decimalType>(bounds.min[0],pos[0]);
        bounds.min[1] = std::min<decimalType>(bounds.min[1],pos[1]);
        bounds.min[2] = std::min<decimalType>(bounds.min[2],pos[2]);
        
        radius = std::max(radius, glm::distance(pos, glm::vec3(0,0,0)));
    }
    
    if (options.uploadToGPU){
	
        //copy out of intermediate
        auto& i = allMeshes.indices;
		totalVerts = allMeshes.NumVerts();;
        totalIndices = i.size();
#if !RVE_SERVER
		meshAllocation = GetApp()->GetRenderEngine().AllocateMesh(allMeshes);
#endif
    }
}
