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
#if !RVE_SERVER
    MeshMaterial mat;
#endif
    Ref<SkeletonAsset> skeleton;
#if !RVE_SERVER
	void updateMaterialInWorldRenderData(MeshMaterial newMat);
#endif
public:
	
	SkinnedMeshComponent(entity_t owner, Ref<SkeletonAsset> sk, Ref<MeshAssetSkinned> mesh) : ComponentWithOwner(owner), skeleton(sk), mesh(mesh){}
#if !RVE_SERVER
	inline void SetMaterial(MeshMaterial newMat){
		updateMaterialInWorldRenderData(newMat);
		mat = newMat;
	}
	/**
	 @returns the currently assigned material
	 */
	inline auto GetMaterial() const{
		return mat;
	}
#endif
	
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
