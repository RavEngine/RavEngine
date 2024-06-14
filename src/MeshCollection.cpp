#if !RVE_SERVER
#include "MeshCollection.hpp"

namespace RavEngine {
	MeshCollectionStatic::MeshCollectionStatic(std::span<Entry> meshes)
	{
		Reserve(meshes.size());
		for (const auto& m : meshes) {
			AddMesh(m);
		}
	}
	void MeshCollectionStatic::AddMesh(const Entry& m)
	{
		meshes.push_back(m.mesh);
		lodDistances.push_back(m.maxDistance);
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
		lodDistances.resize(size);
	}


	MeshCollectionSkinned::MeshCollectionSkinned(const Entry& m)
	{
		meshes.push_back(m.mesh);
	}
}
#endif