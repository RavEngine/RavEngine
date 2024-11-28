#pragma once
#include "ComponentWithOwner.hpp"
#include "Queryable.hpp"
#include "SkeletonAsset.hpp"
#include "Ref.hpp"
#include "BuiltinMaterials.hpp"

namespace RavEngine {

	struct MeshCollectionSkinned;

class SkinnedMeshComponent : public ComponentWithOwner, public Queryable<SkinnedMeshComponent>, public Disableable{
private:
    Ref<MeshCollectionSkinned> mesh;
#if !RVE_SERVER
	Ref<MaterialInstance> mat;
#endif
    Ref<SkeletonAsset> skeleton;
#if !RVE_SERVER
	void updateMaterialInWorldRenderData(decltype(mat) newMat);
#endif
public:
	
	SkinnedMeshComponent(Entity owner, Ref<SkeletonAsset> sk, Ref<MeshCollectionSkinned> mesh) : ComponentWithOwner(owner), skeleton(sk), mesh(mesh){}
#if !RVE_SERVER
	inline void SetMaterial(decltype(mat) newMat){
		updateMaterialInWorldRenderData(newMat);
		mat = newMat;
	}
	/**
	 @returns the currently assigned material
	 */
	auto GetMaterial() const{
		return mat;
	}
#endif
	
    MOVE_NO_COPY(SkinnedMeshComponent);
    
	auto GetMesh() const{
		return mesh;
	}

	Ref<SkeletonAsset> GetSkeleton() const {
		return skeleton;
	}
	
	// shadow Disableable::SetEnabled
	void SetEnabled(bool);
};

}
