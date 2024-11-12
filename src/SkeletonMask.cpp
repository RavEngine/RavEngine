
#include "SkeletonMask.hpp"
#include "SkeletonAsset.hpp"

namespace RavEngine{
SkeletonMask::SkeletonMask(Ref<SkeletonAsset> asset, float defaultValue) : SkeletonMask(asset->GetSkeleton()->num_soa_joints(), defaultValue){
    
}

SkeletonMask::SkeletonMask(uint16_t numBones_soa, float defaultValue){
    mask.resize(numBones_soa, ozz::math::SimdFloat4{ defaultValue,defaultValue,defaultValue,defaultValue });
}

void SkeletonMask::SetMaskForJoint(uint16_t joint_idx, float value){
    float* vec = reinterpret_cast<float*>(&mask[joint_idx / 4]);
    vec[joint_idx % 4] = value;
}

float SkeletonMask::GetMaskForJoint(uint16_t joint_idx) const{
    const float* vec = reinterpret_cast<const float*>(&mask[joint_idx / 4]);
    return  vec[joint_idx % 4];
}

}
