
#include "SkeletonMask.hpp"
#include "SkeletonAsset.hpp"

namespace RavEngine{
SkeletonMask::SkeletonMask(Ref<SkeletonAsset> asset) : SkeletonMask(asset->GetSkeleton()->num_soa_joints()){
    
}

SkeletonMask::SkeletonMask(uint16_t numBones_soa){
    mask.resize(numBones_soa, ozz::math::SimdFloat4{1,1,1,1});
}

void SkeletonMask::SetMaskForJoint(uint16_t joint_idx, float value){
    mask[joint_idx / 4][joint_idx % 4] = value;
}

float SkeletonMask::GetMaskForJoint(uint16_t joint_idx) const{
    return  mask[joint_idx / 4][joint_idx % 4];
}

}
