#pragma once

#include "MeshAsset.hpp"
#include "SkeletonAsset.hpp"
#include "Ref.hpp"
#include <vector>
#include <etl/vector.h>

namespace RavEngine{
class MeshAssetSkinned : public MeshAsset{
public:
	struct vweights {
		struct vw {
			uint16_t joint_idx = 0;
			float influence = 0;
		};
		etl::vector<vw, 4> weights;
	};
private:
	std::vector<vweights> allweights;
public:
	
	MeshAssetSkinned(const std::string& path, Ref<SkeletonAsset> skeleton, float scale = 1.0);
	
	//disable
	MeshAssetSkinned(const std::string& name, const decimalType scale = 1.0) = delete;
	
	inline const decltype(allweights)& getWeights() const{
		return allweights;
	}
};

}
