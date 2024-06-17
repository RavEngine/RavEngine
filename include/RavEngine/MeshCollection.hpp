#pragma once
#if !RVE_SERVER
#include "Ref.hpp"
#include "DataStructures.hpp"
#include "VRAMVector.hpp"

namespace RavEngine {

	class MeshAsset;
	class MeshAssetSkinned;

	template<typename T>
	struct MeshCollection {
		Vector<Ref<T>> meshes;
		struct Entry {
			Ref<T> mesh;
			float maxDistance = 0;
		};
	};

	struct MeshCollectionStatic : protected MeshCollection<MeshAsset> {
		
		MeshCollectionStatic() {}
		MeshCollectionStatic(std::span<Entry> meshes);
		MeshCollectionStatic(std::initializer_list<Entry> meshes);
		MeshCollectionStatic(Ref<MeshAsset> mesh);

		void AddMesh(const Entry& m);

		void RemoveMeshAtIndex(uint16_t idx);

		void Reserve(uint16_t size);
		void Resize(uint16_t size);

		auto GetNumLods() const {
			return lodDistances.size();
		}

		auto GetMeshForLOD(uint32_t i) const {
			return meshes[i];
		}

		float GetRadius() const;

	private:
		VRAMVector<float> lodDistances;
	};

	struct MeshCollectionSkinned : protected MeshCollection<MeshAssetSkinned> {
		MeshCollectionSkinned(const Entry& m);
	};

}
#endif