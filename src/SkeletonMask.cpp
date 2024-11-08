
#include "SkeletonMask.hpp"
#include "SkeletonAsset.hpp"

namespace RavEngine{
SkeletonMask::SkeletonMask(Ref<SkeletonAsset> asset) : SkeletonMask(asset->GetSkeleton()->num_joints()){
    
}

SkeletonMask::SkeletonMask(uint16_t numBones){
    mask.resize(numBones, true);
}

}
