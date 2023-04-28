#pragma once
#include "MeshAsset.hpp"
#include "SkeletonAsset.hpp"
#include "Ref.hpp"
#include "Manager.hpp"

namespace RavEngine{
class MeshAssetSkinned : public MeshAsset{
public:
	struct vweights {
		struct vw {
			float joint_idx = 0;
			float influence = 0;
		};
		FixedVector<vw, 4> weights;
	};
private:
#if 0
	bgfx::VertexBufferHandle weightsHandle = BGFX_INVALID_HANDLE;
#endif
public:
	
	~MeshAssetSkinned(){
	}
	
	MeshAssetSkinned(const std::string& path, Ref<SkeletonAsset> skeleton, float scale = 1.0);
	
	//disable
	MeshAssetSkinned(const std::string& name, const decimalType scale = 1.0) = delete;
    
    // use this to load assets
    struct Manager : public GenericWeakReadThroughCache<std::string,MeshAssetSkinned>{};
};

}
