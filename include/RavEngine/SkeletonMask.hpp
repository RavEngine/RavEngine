#pragma once
#include "Ref.hpp"
#include "Vector.hpp"
#include <ozz/base/maths/simd_math.h>

namespace RavEngine{

struct SkeletonAsset;

struct SkeletonMask{
    friend class AnimatorComponent;
    
    SkeletonMask(Ref<SkeletonAsset> asset);
    SkeletonMask(uint16_t numBones);
    
    void SetMaskForJoint(uint16_t joint_idx, float value);
    
    float GetMaskForJoint(uint16_t joint_idx) const;
    
    uint16_t GetNumPackedJoints() const{
        return mask.size();
    }
    
private:
    Vector<ozz::math::SimdFloat4> mask;    
};

}
