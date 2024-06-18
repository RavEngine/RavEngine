#pragma once
#if !RVE_SERVER
#include "Ref.hpp"
#include "DataStructures.hpp"
#include "VRAMVector.hpp"
#include "MeshAsset.hpp"
#include "MeshAssetSkinned.hpp"

namespace RavEngine {

	class MeshAssetSkinned;
	struct MeshRange;
	struct SkeletonAsset;

	template<typename T>
	struct MeshCollection {
		Vector<Ref<T>> meshes;
		struct Entry {
			Ref<T> mesh;
			float minDistance = 0;	// the minimum distance from the object to the camera for this LOD to be usable
		};
	};

	struct MeshCollectionStatic : public MeshCollection<MeshAsset> {
		friend class RenderEngine;
		MeshCollectionStatic() {}
		MeshCollectionStatic(std::span<Entry> meshes);
		MeshCollectionStatic(std::initializer_list<Entry> meshes);
		MeshCollectionStatic(Ref<MeshAsset> mesh);
		MeshCollectionStatic(const std::string& meshName, const MeshAssetOptions& opt = {}) : MeshCollectionStatic(MeshAsset::Manager::Get(meshName, opt)) {}

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

		void SetMeshForLOD(uint32_t i, Ref<MeshAsset> mesh) {
			meshes.at(i) = mesh;
		}

		float GetRadius() const;

	private:
		VRAMVector<float> lodDistances;
	};

	struct MeshCollectionSkinned : protected MeshCollection<MeshAssetSkinned> {
	friend class RenderEngine;
		MeshCollectionSkinned(const Entry& m);
		MeshCollectionSkinned(const std::string& name, Ref<SkeletonAsset> skeleton, float scale = 1.0) : MeshCollectionSkinned({ MeshAssetSkinned::Manager::Get(name,skeleton, scale) }) {}

		uint32_t GetNumVerts() const;
		uint32_t GetNumIndices() const;
		uint32_t GetNumLods() const;
		float GetRadius() const;
		RGLBufferPtr GetWeightsBuffer() const;
		MeshRange GetAllocation() const;

	private:
		VRAMVector<float> lodDistances;
	};

	struct MeshCollectionStaticManager : public GenericWeakReadThroughCache<std::string, MeshCollectionStatic> {};
	struct MeshCollectionSkinnedManager : public GenericWeakReadThroughCache<std::string, MeshCollectionSkinned> {};

}
#endif