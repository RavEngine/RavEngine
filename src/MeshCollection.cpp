#if !RVE_SERVER
#include "MeshCollection.hpp"
#include "MeshAsset.hpp"
#include "MeshAssetSkinned.hpp"
#include "MeshAllocation.hpp"

namespace RavEngine {
	MeshCollectionStatic::MeshCollectionStatic(std::span<Entry> meshes)
	{
		Reserve(meshes.size());
		for (const auto& m : meshes) {
			AddMesh(m);
		}
	}
	MeshCollectionStatic::MeshCollectionStatic(std::initializer_list<Entry> meshes) 
	{
		Reserve(meshes.size());
		for (const auto& m : meshes) {
			AddMesh(m);
		}
	}
	MeshCollectionStatic::MeshCollectionStatic(Ref<MeshAsset> mesh)
	{
		AddMesh({mesh, std::numeric_limits<float>::infinity()});
	}

	void MeshCollectionStatic::AddMesh(const Entry& m)
	{
		meshes.push_back(m.mesh);
		lodDistances.push_back(m.minDistance);
		auto attrCheck = m.mesh->GetAttributes();
		for (const auto& mesh : meshes) {
			Debug::Assert(attrCheck == mesh->GetAttributes(), "Mesh attributes do not match!");
		}
	}
	void MeshCollectionStatic::RemoveMeshAtIndex(uint16_t idx)
	{
		meshes.erase(meshes.begin() + idx);
		lodDistances.erase(lodDistances.begin() + idx);
	}

	void MeshCollectionStatic::Reserve(uint16_t size)
	{
		meshes.reserve(size);
		lodDistances.reserve(size);
	}

	void MeshCollectionStatic::Resize(uint16_t size)
	{
		meshes.resize(size);
		lodDistances.Resize(size);
	}

	float MeshCollectionStatic::GetRadius() const
	{
		Debug::Assert(meshes.size() > 0, "Mesh collection is empty!");
		return meshes.front()->GetRadius();
	}


	MeshCollectionSkinned::MeshCollectionSkinned(const Entry& m)
	{
		meshes.push_back(m.mesh);
		lodDistances.push_back(std::numeric_limits<float>::infinity());
	}

	uint32_t MeshCollectionSkinned::GetNumVerts() const
	{
		return meshes.front()->GetNumVerts();
	}
	uint32_t MeshCollectionSkinned::GetNumIndices() const
	{
		return meshes.front()->GetNumIndices();
	}
	uint32_t MeshCollectionSkinned::GetNumLods() const
	{
		return meshes.size();
	}
	float MeshCollectionSkinned::GetRadius() const
	{
		return meshes.front()->GetRadius();
	}
	RGLBufferPtr MeshCollectionSkinned::GetWeightsBuffer() const
	{
		return meshes.front()->GetWeightsBuffer();
	}
	MeshRange MeshCollectionSkinned::GetAllocation() const
	{
		return meshes.front()->GetAllocation();
	}
}
#endif
