#pragma once
#include "MeshAsset.hpp"
#include "SkeletonAsset.hpp"
#include "Ref.hpp"
#include <bgfx/bgfx.h>
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
	bgfx::VertexBufferHandle weightsHandle = BGFX_INVALID_HANDLE;
public:
	
	~MeshAssetSkinned(){
		bgfx::destroy(weightsHandle);
	}
	
	MeshAssetSkinned(const std::string& path, Ref<SkeletonAsset> skeleton, float scale = 1.0);
	
	//disable
	MeshAssetSkinned(const std::string& name, const decimalType scale = 1.0) = delete;
    
    // use this to load assets
    struct Manager : public GenericWeakReadThroughCache<std::string,MeshAssetSkinned>{};
	
    constexpr inline const decltype(weightsHandle) GetWeightsHandle() const{
		return weightsHandle;
	}
};

}
