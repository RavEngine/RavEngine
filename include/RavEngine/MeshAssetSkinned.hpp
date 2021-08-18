#pragma once

#include "MeshAsset.hpp"
#include "SkeletonAsset.hpp"
#include "Ref.hpp"
#include <vector>
#include <etl/vector.h>
#include <bgfx/bgfx.h>

namespace RavEngine{
class MeshAssetSkinned : public MeshAsset{
public:
	struct vweights {
		struct vw {
			float joint_idx = 0;
			float influence = 0;
		};
		etl::vector<vw, 4> weights;
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

	
	inline const decltype(weightsHandle) GetWeightsHandle() const{
		return weightsHandle;
	}
};

}
