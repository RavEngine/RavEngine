#pragma once
#include "Component.hpp"
#include "RenderableComponent.hpp"
#include "Queryable.hpp"
#include "SkeletonAsset.hpp"
#include "Ref.hpp"
#include "MeshAssetSkinned.hpp"
#include "BuiltinMaterials.hpp"

namespace RavEngine {

class SkinnedMeshComponent : public RenderableComponent, public QueryableDelta<RenderableComponent, SkinnedMeshComponent>{
private:
	Ref<SkeletonAsset> skeleton;
	Ref<MeshAssetSkinned> meshAsset;
	Ref<PBRMaterialInstance> material;
	
public:
	using QueryableDelta<RenderableComponent, SkinnedMeshComponent>::GetQueryTypes;
	
	SkinnedMeshComponent(decltype(skeleton) sk, decltype(meshAsset) mesh) : skeleton(sk), meshAsset(mesh){}
	
	inline void SetMaterial(decltype(material) newMat){
		material = newMat;
	}
		
	/**
	 @returns the currently assigned material
	 */
	inline decltype(material) GetMaterial() const{
		return material;
	}
	
	inline decltype(meshAsset) GetMesh() const{
		return meshAsset;
	}

	inline decltype(skeleton) GetSkeleton() const {
		return skeleton;
	}
};

}
