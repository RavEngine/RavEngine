#pragma once
#include "MeshAsset.hpp"
#include "Ref.hpp"
#include "Manager.hpp"
#include <RGL/Types.hpp>

namespace RavEngine{
	class SkeletonAsset;

class MeshAssetSkinned : public MeshAsset{
public:
	struct vweights {
		struct vw {
			uint32_t joint_idx = 0;
			float influence = 0;
		};
		FixedVector<vw, 4> weights;
	};
private:
	RGLBufferPtr weightsBuffer;
public:
	
	~MeshAssetSkinned();
	
	MeshAssetSkinned(const std::string& path, Ref<SkeletonAsset> skeleton, float scale = 1.0);
	
	//disable
	MeshAssetSkinned(const std::string& name, const decimalType scale = 1.0) = delete;
    
    // use this to load assets
    struct Manager : public GenericWeakReadThroughCache<std::string,MeshAssetSkinned>{};

	 auto GetWeightsBuffer() const {
		 return weightsBuffer;
	}
};

}
