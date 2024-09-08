#pragma once
#include "MeshAsset.hpp"
#include "Ref.hpp"
#include "Manager.hpp"
#if !RVE_SERVER
#include <RGL/Types.hpp>
#endif

namespace RavEngine{
	class SkeletonAsset;

class MeshAssetSkinned : public MeshAsset{

private:
#if !RVE_SERVER
	RGLBufferPtr weightsBuffer;
#endif
public:
	
	~MeshAssetSkinned();
	
	MeshAssetSkinned(const std::string& path, Ref<SkeletonAsset> skeleton, float scale = 1.0);
	
	//disable
	MeshAssetSkinned(const std::string& name, const decimalType scale = 1.0) = delete;
    
    // use this to load assets
    struct Manager : public GenericWeakReadThroughCache<std::string,MeshAssetSkinned>{};
#if !RVE_SERVER
	 auto GetWeightsBuffer() const {
		 return weightsBuffer;
	}
#endif
};

}
