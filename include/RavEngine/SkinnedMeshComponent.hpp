#pragma once
#include "Component.hpp"
#include "Queryable.hpp"
#include "SkeletonAsset.hpp"
#include "Ref.hpp"
#include "MeshAssetSkinned.hpp"
#include "BuiltinMaterials.hpp"

namespace RavEngine {

class SkinnedMeshComponent : public Component, public Queryable<SkinnedMeshComponent>{
private:
	std::tuple<Ref<MeshAssetSkinned>, Ref<MaterialInstanceBase>,Ref<SkeletonAsset>> tuple;

public:
	
	SkinnedMeshComponent(Ref<SkeletonAsset> sk, Ref<MeshAssetSkinned> mesh){
		std::get<2>(tuple) = sk;
		std::get<0>(tuple) = mesh;
	}
	
	inline void SetMaterial(Ref<PBRMaterialInstance> newMat){
		std::get<1>(tuple) = newMat;
	}
		
	/**
	 @returns the currently assigned material
	 */
	inline Ref<MaterialInstanceBase> GetMaterial() const{
		return std::get<1>(tuple);
	}
	
	inline Ref<MeshAssetSkinned> GetMesh() const{
		return std::get<0>(tuple);
	}

	inline Ref<SkeletonAsset> GetSkeleton() const {
		return std::get<2>(tuple);
	}
	
    constexpr inline const decltype(tuple)& getTuple() const{
		return tuple;
	}
};

}
