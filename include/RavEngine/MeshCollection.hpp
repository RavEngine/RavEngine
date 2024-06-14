#pragma once
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

		void AddMesh(const Entry& m);

		void RemoveMeshAtIndex(uint16_t idx);

		void Reserve(uint16_t size);
		void Resize(uint16_t size);

	private:
		VRAMVector<float> lodDistances;
	};

	struct MeshCollectionSkinned : protected MeshCollection<MeshAssetSkinned> {
		MeshCollectionSkinned(const Entry& m);
	};

}