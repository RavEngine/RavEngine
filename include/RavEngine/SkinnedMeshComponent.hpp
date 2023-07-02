#pragma once
#include "ComponentWithOwner.hpp"
#include "Queryable.hpp"
#include "SkeletonAsset.hpp"
#include "Ref.hpp"
#include "MeshAssetSkinned.hpp"
#include "BuiltinMaterials.hpp"

namespace RavEngine {

class SkinnedMeshComponent : public ComponentWithOwner, public Queryable<SkinnedMeshComponent>, public Disableable{
private:
    Ref<MeshAssetSkinned> mesh;
    Ref<MaterialInstance> mat;
    Ref<SkeletonAsset> skeleton;
	void updateMaterialInWorldRenderData(Ref<MaterialInstance> newMat);
public:
	
	SkinnedMeshComponent(entity_t owner, Ref<SkeletonAsset> sk, Ref<MeshAssetSkinned> mesh) : ComponentWithOwner(owner), skeleton(sk), mesh(mesh){}
	
	inline void SetMaterial(Ref<MaterialInstance> newMat){
		updateMaterialInWorldRenderData(newMat);
		mat = newMat;
	}
		
	/**
	 @returns the currently assigned material
	 */
	inline auto GetMaterial() const{
		return mat;
	}
	
	inline Ref<MeshAssetSkinned> GetMesh() const{
		return mesh;
	}

	inline Ref<SkeletonAsset> GetSkeleton() const {
		return skeleton;
	}
	
	// shadow Disableable::SetEnabled
	void SetEnabled(bool);
};

}
