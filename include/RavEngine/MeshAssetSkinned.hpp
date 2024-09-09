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
	
	MeshAssetSkinned(const std::string& path);
	    
    // use this to load assets
    struct Manager : public GenericWeakReadThroughCache<std::string,MeshAssetSkinned>{};
#if !RVE_SERVER
	 auto GetWeightsBuffer() const {
		 return weightsBuffer;
	}
#endif
};

}
